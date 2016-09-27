#include <jni.h>
#include <android/log.h>
#include "bugsnag.h"

// Platform specific libraries
#include <signal.h>
#include <stdio.h>
#include <dlfcn.h>
#include <unwind.h>
#include <time.h>

// Globals
/* signals to be handled */
static const int native_sig_catch[SIG_CATCH_COUNT + 1]
        = { SIGILL, SIGTRAP, SIGABRT, SIGBUS, SIGFPE, SIGSEGV };

/* the Bugsnag signal handler */
struct sigaction *g_sigaction;

/* the old signal handler array */
struct sigaction *g_sigaction_old;

/* the pre-populated Bugsnag error */
struct bugsnag_error *g_bugsnag_error;

/* structure for storing the unwound stack trace */
static native_code_handler_struct *g_native_code;

/**
 * Unwinds the stack trace for given context
 */
static _Unwind_Reason_Code unwind_callback(struct _Unwind_Context* context, void* arg)
{
    struct backtrace_state* state = (struct backtrace_state*)arg;
    uintptr_t pc = _Unwind_GetIP(context);
    if (pc) {
        if (state->current == state->end) {
            return _URC_END_OF_STACK;
        } else {
            *state->current++ = (void*)pc;
        }
    }
    return _URC_NO_REASON;
}

/**
 * Gets the stack trace
 */
size_t capture_backtrace(void** buffer, size_t max)
{
    struct backtrace_state state = {buffer, buffer + max};
    _Unwind_Backtrace(unwind_callback, &state);

    return state.current - buffer;
}

/**
 * Gets a string representation of the error code
 */
char* get_signal_name(int code) {
    if (code == SIGILL ) {
        return "SIGILL";
    } else if (code == SIGTRAP ) {
        return "SIGTRAP";
    } else if (code == SIGABRT ) {
        return "SIGABRT";
    } else if (code == SIGBUS ) {
        return "SIGBUS";
    } else if (code == SIGFPE ) {
        return "SIGFPE";
    } else if (code == SIGSEGV ) {
        return "SIGSEGV";
    } else {
        return "UNKNOWN";
    }
}

/**
 * writes the given stack trace frame to the given file
 */
static void output_stack_frame(struct bugsnag_stack_frame frame, FILE* file) {
    fputs("{\"method\":\"", file);
    if (frame.method == NULL) {
        fputs("(null)", file);
    } else {
        fputs(frame.method, file);
    }

    fputs("\",\"file\":\"", file);
    fputs(frame.file, file);
    fputs("\",\"lineNumber\":", file);
    fprintf(file, "%d", frame.line_number);
    fputs("}", file);
}

/**
 * writes the given exception to the given file
 */
static void output_exception(struct bugsnag_exception ex, FILE* file) {
    fputs("\"errorClass\":\"", file);
    fputs(ex.error_class, file);
    fputs("\",\"message\":\"", file);
    fputs(ex.message, file);
    fputs("\",\"stacktrace\":[", file);

    int i;
    for (i = 0; i < ex.frames_used; i++) {
        output_stack_frame(ex.stack_trace[i], file);

        if (i < ex.frames_used -1) {
            fputs(",", file);
        }
    }

    fputs("]", file);
}

/**
 * writes the given user to the given file
 */
static void output_user(struct bugsnag_user user, FILE* file) {
    if (strlen(user.id) > 1) {
        fputs("\"id\":\"",file);
        fputs(user.id, file);
        fputs("\"", file);
    }

    if (strlen(user.email) > 1) {
        if (strlen(user.id) > 1) {
            fputs(",", file);
        }

        fputs("\"email\":\"",file);
        fputs(user.email, file);
        fputs("\"", file);
    }

    if (strlen(user.name) > 1) {
        if (strlen(user.id) > 1 || strlen(user.email) > 1) {
            fputs(",", file);
        }

        fputs("\"name\":\"",file);
        fputs(user.name, file);
        fputs("\"", file);
    }
}

/**
 * writes the given app data to the given file
 */
static void output_app_data(struct bugsnag_app_data app, FILE* file) {
    fputs("\"id\":\"", file);
    fputs(app.package_name, file);

    fputs("\",\"name\":\"", file);
    fputs(app.app_name, file);

    fputs("\",\"packageName\":\"", file);
    fputs(app.package_name, file);

    fputs("\",\"versionName\":\"", file);
    fputs(app.version_name, file);

    fputs("\",\"versionCode\":\"", file);
    fprintf(file, "%d", app.version_code);

    if (strlen(app.build_uuid) > 1) {
        fputs("\",\"buildUUID\":\"", file);
        fputs(app.build_uuid, file);
    }

    fputs("\",\"version\":\"", file);
    fputs(app.version, file);

    fputs("\",\"releaseStage\":\"", file);
    fputs(app.release_stage, file);
    fputs("\"", file);
}

/**
 * writes the given device data to the given file
 */
static void output_device(struct bugsnag_device device, FILE* file) {
    fputs("\"osName\":\"", file);
    fputs(device.os_name, file);

    fputs("\",\"manufacturer\":\"", file);
    fputs(device.manufacturer, file);

    fputs("\",\"brand\":\"", file);
    fputs(device.brand, file);

    fputs("\",\"model\":\"", file);
    fputs(device.model, file);

    fputs("\",\"id\":\"", file);
    fputs(device.id, file);

    fputs("\",\"apiLevel\":\"", file);
    fprintf(file, "%d", device.api_level);

    fputs("\",\"osVersion\":\"", file);
    fputs(device.os_version, file);

    fputs("\",\"osBuild\":\"", file);
    fputs(device.os_build, file);

    fputs("\",\"locale\":\"", file);
    fputs(device.locale, file);

    fputs("\",\"totalMemory\":\"", file);
    fprintf(file, "%g", device.total_memory);

    fputs("\",\"jailbroken\":\"", file);
    fputs(device.rooted, file);

    fputs("\",\"screenDensity\":\"", file);
    fprintf(file, "%g", device.screen_density);

    fputs("\",\"dpi\":\"", file);
    fprintf(file, "%d", device.dpi);

    fputs("\",\"screenResolution\":\"", file);
    fputs(device.screen_resolution, file);
    fputs("\"", file);
}

/**
 * writes the given error to the given file
 */
static void output_error(struct bugsnag_error er, FILE* file) {

    fputs("{ \"payloadVersion\":\"", file);
    fputs(er.payload_version, file);

    if (strlen(er.context) > 1) {
        fputs("\",\"context\":\"",file);
        fputs(er.context, file);
    }

    fputs("\",\"severity\":\"",file);
    fputs(er.severity, file);

    fputs("\",\"exceptions\":[{", file);
    output_exception(g_bugsnag_error->exception, file);
    fputs("}]", file);

    if (strlen(er.user.id) > 1 || strlen(er.user.email) > 1 || strlen(er.user.name) > 1) {
        fputs(",\"user\":{", file);
        output_user(er.user, file);
        fputs("}", file);
    }

    fputs(",\"app\": {",file);
    output_app_data(er.app_data, file);
    fputs("}", file);

    fputs(",\"device\": {",file);
    output_device(er.device, file);
    fputs("}", file);

    fputs("}", file);
}

/**
 * Checks to see if the given string starts with the given prefix
 */
int startsWith(const char *pre, const char *str)
{
    size_t lenpre = strlen(pre);
    size_t lenstr = strlen(str);

    if (lenstr < lenpre) {
        return 0; // false
    } else {
        return strncmp(pre, str, lenpre) == 0;
    }
}

/**
 * Handles signals when errors occur and writes a file to the Bugsnag error cache
 */
static void signal_handler(int code, struct siginfo* si, void* sc) {
    __android_log_print(ANDROID_LOG_VERBOSE, "BugsnagNdk", "In signal_handler with signal %d", si->si_signo);

    int frames_size = (int)capture_backtrace(g_native_code->uframes, FRAMES_MAX);

    // Create an exception message and class
    sprintf(g_bugsnag_error->exception.message,"Fatal signal from native: %d (%s), code %d", si->si_signo, get_signal_name(si->si_signo), si->si_code);
    sprintf(g_bugsnag_error->exception.error_class,"Native Error: %s", get_signal_name(si->si_signo));

    // Ignore the first 2 frames (handler code)
    int project_frames = frames_size - FRAMES_TO_IGNORE;
    int frames_used = 0;

    if (project_frames > 0) {

        // populate stack frame element array
        int i;
        for (i = 0; i < project_frames; i++) {
            void *const addr = g_native_code->uframes[i + FRAMES_TO_IGNORE];

            Dl_info info;
            if (dladdr(addr, &info) != 0 && info.dli_fname != NULL) {

                // Check that this isn't a system file
                if (!startsWith("/system/", info.dli_fname)) {

                    // Attempt to calculate the line numbers TODO: this seems to produce slightly incorrect results
                    uintptr_t near = (uintptr_t) info.dli_saddr;
                    uintptr_t offs = (uintptr_t) g_native_code->uframes[i + FRAMES_TO_IGNORE] - near;

                    g_bugsnag_error->exception.stack_trace[frames_used].file = info.dli_fname;
                    g_bugsnag_error->exception.stack_trace[frames_used].method = info.dli_sname;
                    g_bugsnag_error->exception.stack_trace[frames_used].line_number = (int)offs;

                    frames_used++;
                }
            }
        }
    }
    g_bugsnag_error->exception.frames_used = frames_used;

    // Create a filename for the error
    time_t now = time(NULL);
    char filename[strlen(g_bugsnag_error->error_store_path) + 20];
    sprintf(filename, "%s%ld.json", g_bugsnag_error->error_store_path, (now * 1000));
    FILE* file = fopen(filename, "w+");

    if (file != NULL)
    {
        output_error(*g_bugsnag_error, file);

        fflush(file);
        fclose(file);
    }

    /* Call previous handler. */
    if (si->si_signo >= 0 && si->si_signo < SIG_NUMBER_MAX) {
        g_sigaction_old[si->si_signo].sa_sigaction(code, si, sc);
    }
}

/**
 * Gets the value from a method that returns a string
 */
const char *get_method_string(JNIEnv *env,
                              jclass class,
                              jobject object,
                              const char *method_name) {
    jmethodID method = (*env)->GetMethodID(env, class, method_name, "()Ljava/lang/String;");
    jstring value = (*env)->CallObjectMethod(env, object, method);

    if (value == NULL) {
        return "";
    } else {
        return (*env)->GetStringUTFChars(env, value, JNI_FALSE);
    }
}

/**
 * Gets the value from a field that contains a string
 */
const char *get_field_string(JNIEnv *env,
                             jclass class,
                             jobject object,
                             const char *field_name) {
    jfieldID field = (*env)->GetFieldID(env, class, field_name, "Ljava/lang/String;");
    jstring value = (*env)->GetObjectField(env, object, field);

    if (value == NULL) {
        return "";
    } else {
        return (*env)->GetStringUTFChars(env, value, JNI_FALSE);
    }
}

/**
 * Calls a static method called "getString" with the given param on the given class to get a string value
 */
const char *get_build_property_string(JNIEnv *env,
                                      jclass class,
                                      const char *param_name) {

    jmethodID method = (*env)->GetStaticMethodID(env, class, "getString", "(Ljava/lang/String;)Ljava/lang/String;");
    jstring param = (*env)->NewStringUTF(env, param_name);
    jstring value = (*env)->CallStaticObjectMethod(env, class, method, param);

    if (value == NULL) {
        return "";
    } else {
        return (*env)->GetStringUTFChars(env, value, JNI_FALSE);
    }
}

/**
 * Gets the value from a field that contains a java.lang.integer
 */
int get_field_int(JNIEnv *env,
                  jclass class,
                  jobject object,
                  const char *field_name) {
    jfieldID field = (*env)->GetFieldID(env, class, field_name, "Ljava/lang/Integer;");
    jobject value_integer = (*env)->GetObjectField(env, object, field);

    jclass integer_class = (*env)->FindClass(env, "java/lang/Integer");
    jmethodID get_value_method = (*env)->GetMethodID(env, integer_class, "intValue", "()I");
    jint value = (*env)->CallIntMethod(env, value_integer, get_value_method);

    if (value == NULL) {
        return 0;
    } else {
        return (int)value;
    }
}

/**
 * Gets the value from a field that contains a java.lang.float
 */
float get_field_float(JNIEnv *env,
                      jclass class,
                      jobject object,
                      const char *field_name) {
    jfieldID field = (*env)->GetFieldID(env, class, field_name, "Ljava/lang/Float;");
    jobject value_float = (*env)->GetObjectField(env, object, field);

    jclass float_class = (*env)->FindClass(env, "java/lang/Float");
    jmethodID get_value_method = (*env)->GetMethodID(env, float_class, "floatValue", "()F");
    jfloat value = (*env)->CallFloatMethod(env, value_float, get_value_method);

    return (float)value;
}

/**
 * Gets the value from a field that contains a java.lang.long
 */
double get_field_long(JNIEnv *env,
                      jclass class,
                      jobject object,
                      const char *field_name) {
    jfieldID field = (*env)->GetFieldID(env, class, field_name, "Ljava/lang/Long;");
    jobject value_long = (*env)->GetObjectField(env, object, field);

    jclass long_class = (*env)->FindClass(env, "java/lang/Long");
    jmethodID get_value_method = (*env)->GetMethodID(env, long_class, "doubleValue", "()D");
    jdouble value = (*env)->CallDoubleMethod(env, value_long, get_value_method);

    return (double)value;
}

/**
 * Gets the value from a field that contains a java.lang.boolean
 * returns "true" or "false" string
 */
const char* get_field_boolean(JNIEnv *env,
                              jclass class,
                              jobject object,
                              const char *field_name) {
    jfieldID field = (*env)->GetFieldID(env, class, field_name, "Ljava/lang/Boolean;");
    jobject value_boolean = (*env)->GetObjectField(env, object, field);

    jclass boolean_class = (*env)->FindClass(env, "java/lang/Boolean");
    jmethodID get_string_method = (*env)->GetMethodID(env, boolean_class, "toString", "()Ljava/lang/String;");
    jstring value = (*env)->CallObjectMethod(env, value_boolean, get_string_method);

    if (value == NULL) {
        return "";
    } else {
        return (*env)->GetStringUTFChars(env, value, JNI_FALSE);
    }
}

/**
 * Gets the user details from the client class and pre-populates the bugsnag error
 */
void populate_user_details(JNIEnv *env, jclass client_class, jobject client) {
    // Get the user information
    jfieldID get_user_field = (*env)->GetFieldID(env, client_class, "user", "Lcom/bugsnag/android/User;");
    jobject user = (*env)->GetObjectField(env, client, get_user_field);

    g_bugsnag_error->user.id = "";
    g_bugsnag_error->user.email = "";
    g_bugsnag_error->user.name = "";

    if (user != NULL) {
        jclass user_class = (*env)->FindClass(env, "com/bugsnag/android/User");

        g_bugsnag_error->user.id = get_method_string(env, user_class, user, "getId");
        g_bugsnag_error->user.email = get_method_string(env, user_class, user, "getEmail");
        g_bugsnag_error->user.name = get_method_string(env, user_class, user, "getName");
    }
}

/**
 * Gets the app data details from the client class and pre-populates the bugsnag error
 */
void populate_app_data(JNIEnv *env, jclass client_class, jobject client) {
    // Get the App Data
    jfieldID get_app_data_field = (*env)->GetFieldID(env, client_class, "appData", "Lcom/bugsnag/android/AppData;");
    jobject app_data = (*env)->GetObjectField(env, client, get_app_data_field);

    if (app_data != NULL) {
        jclass app_data_class = (*env)->FindClass(env, "com/bugsnag/android/AppData");

        g_bugsnag_error->app_data.package_name = get_field_string(env, app_data_class, app_data, "packageName");
        g_bugsnag_error->app_data.app_name = get_field_string(env, app_data_class, app_data, "appName");
        g_bugsnag_error->app_data.version_name = get_field_string(env, app_data_class, app_data, "versionName");
        g_bugsnag_error->app_data.version_code = get_field_int(env, app_data_class, app_data, "versionCode");
        // Build UUID comes from configuration
        g_bugsnag_error->app_data.version = get_method_string(env, app_data_class, app_data, "getAppVersion");
        g_bugsnag_error->app_data.release_stage = get_method_string(env, app_data_class, app_data, "getReleaseStage");
    }

    // Get the configuration
    jfieldID get_config_field = (*env)->GetFieldID(env, client_class, "config", "Lcom/bugsnag/android/Configuration;");
    jobject config = (*env)->GetObjectField(env, client, get_config_field);
    if (config != NULL) {
        jclass config_class = (*env)->FindClass(env, "com/bugsnag/android/Configuration");

        g_bugsnag_error->app_data.build_uuid = get_method_string(env, config_class, config, "getBuildUUID");
    }
}

/**
 * Gets the device data details from the client class and pre-populates the bugsnag error
 */
void populate_device_data(JNIEnv *env, jclass client_class, jobject client) {
    // Get the device data
    g_bugsnag_error->device.os_name = "android";

    jfieldID get_device_data_field = (*env)->GetFieldID(env, client_class, "deviceData", "Lcom/bugsnag/android/DeviceData;");
    jobject device_data = (*env)->GetObjectField(env, client, get_device_data_field);

    if (device_data != NULL) {
        jclass device_data_class = (*env)->FindClass(env, "com/bugsnag/android/DeviceData");

        g_bugsnag_error->device.id = get_field_string(env, device_data_class, device_data, "id");
        g_bugsnag_error->device.locale = get_field_string(env, device_data_class, device_data, "locale");
        g_bugsnag_error->device.total_memory = get_field_long(env, device_data_class, device_data, "totalMemory");
        g_bugsnag_error->device.rooted = get_field_boolean(env, device_data_class, device_data, "rooted");
        g_bugsnag_error->device.screen_density = get_field_float(env, device_data_class, device_data, "screenDensity");
        g_bugsnag_error->device.dpi = get_field_int(env, device_data_class, device_data, "dpi");
        g_bugsnag_error->device.screen_resolution = get_field_string(env, device_data_class, device_data, "screenResolution");
    }

    // Get the android Build class
    jclass build_class = (*env)->FindClass(env, "android/os/Build");

    // TODO: perhaps cache these values in DeviceData so they are easier to access from here
    g_bugsnag_error->device.manufacturer = get_build_property_string(env, build_class, "ro.product.manufacturer");
    g_bugsnag_error->device.brand = get_build_property_string(env, build_class, "ro.product.brand");
    g_bugsnag_error->device.model = get_build_property_string(env, build_class, "ro.product.model");
    g_bugsnag_error->device.os_build = get_build_property_string(env, build_class, "ro.build.display.id");
    g_bugsnag_error->device.os_version = get_build_property_string(env, build_class, "ro.build.version.release");

    // Get the android Build class
    jclass system_properties_class = (*env)->FindClass(env, "android/os/SystemProperties");
    jmethodID method = (*env)->GetStaticMethodID(env, system_properties_class, "getInt", "(Ljava/lang/String;I)I");
    jstring param1 = (*env)->NewStringUTF(env, "ro.build.version.sdk");
    jint param2 = 0;
    jint sdk_int = (*env)->CallStaticIntMethod(env, system_properties_class, method, param1, param2);
    g_bugsnag_error->device.api_level = sdk_int;
}

/**
 * Gets details from java to pre-populates the bugsnag error
 */
void populate_error_details(JNIEnv *env) {

    // Constant values for payload version and severity
    g_bugsnag_error->payload_version = "3";
    g_bugsnag_error->severity = "error";

    // Get the Client object
    jclass bugsnag_class = (*env)->FindClass(env, "com/bugsnag/android/Bugsnag");
    jmethodID get_client_method = (*env)->GetStaticMethodID(env, bugsnag_class, "getClient", "()Lcom/bugsnag/android/Client;");
    jobject client = (*env)->CallStaticObjectMethod(env, bugsnag_class, get_client_method);
    jclass client_class = (*env)->FindClass(env, "com/bugsnag/android/Client");

    // populate the context
    g_bugsnag_error->context = get_method_string(env, client_class, client, "getContext");

    // Get the error store path
    jfieldID error_store_field = (*env)->GetFieldID(env, client_class, "errorStore", "Lcom/bugsnag/android/ErrorStore;");
    jobject error_store = (*env)->GetObjectField(env, client, error_store_field);
    jclass error_store_class = (*env)->FindClass(env, "com/bugsnag/android/ErrorStore");
    g_bugsnag_error->error_store_path = get_field_string(env, error_store_class, error_store, "path");

    populate_user_details(env, client_class, client);

    populate_app_data(env, client_class, client);

    populate_device_data(env, client_class, client);
}

/**
 * Adds the Bugsnag signal handler
 */
int setupBugsnag(JNIEnv *env) {
    g_native_code = calloc(sizeof(native_code_handler_struct), 1);
    g_bugsnag_error = calloc(sizeof(bugsnag_error_struct), 1);

    populate_error_details(env);

    // create a signal handler
    g_sigaction = calloc(sizeof(struct sigaction), 1);
    memset(g_sigaction, 0, sizeof(struct sigaction));
    sigemptyset(&g_sigaction->sa_mask);
    g_sigaction->sa_sigaction = signal_handler;
    g_sigaction->sa_flags = SA_SIGINFO;

    g_sigaction_old = calloc(sizeof(struct sigaction), SIG_NUMBER_MAX);
    int i;
    for (i = 0; i < SIG_CATCH_COUNT; i++) {
        const int sig = native_sig_catch[i];
        sigaction(sig, g_sigaction, &g_sigaction_old[sig]);
    }

    return 0;
}

/**
 * Removes the Bugsnag signal handler
 */
void tearDownBugsnag() {
    // replace signal handler with old one again
    int i;
    for(i = 0; i < SIG_CATCH_COUNT; i++) {
        int sig = native_sig_catch[i];
        sigaction(sig, &g_sigaction_old[sig], NULL);
    }

    free(g_sigaction);
    free(g_native_code);
    free(g_bugsnag_error);
    free(g_sigaction_old);
}

