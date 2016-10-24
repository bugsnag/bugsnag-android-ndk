
#include <jni.h>
#include <stdio.h>
#include <string.h>
#include <android/log.h>
#include "bugsnag_error.h"

/**
 * Removes any path from the filename to make it consistent across API versions
 */
static char* stripPathFromFile(const char* file) {

    char* pos = (char *) file;
    char* newpos = strchr(pos, '/');

    while (newpos) {
        pos = newpos + 1;
        newpos = strchr(pos, '/');
    }

    return pos;
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

    if (frame.method_offset != NULL) {
        fprintf(file, "+%i", frame.method_offset);
    }

    fputs("\",\"file\":\"", file);
    fputs(stripPathFromFile(frame.file), file);

    // TODO: can we get the proper line number client side?
    fputs("\",\"lineNumber\":", file);
    fprintf(file, "%d", frame.file_offset);

    fputs(",\"loadAddress\":", file);
    fprintf(file, "%d", (uintptr_t)frame.file_address);

    if (frame.method_address != NULL) {
        fputs(",\"symbolAddress\":", file);
        fprintf(file, "%d", (uintptr_t) frame.method_address);
    }

    fputs(",\"frameAddress\":", file);
    fprintf(file, "%d", (uintptr_t)frame.frame_address);

    fputs(",\"inProject\":", file);
    if (frame.in_project) {
        fputs("\"true\"", file);
    } else {
        fputs("\"false\"", file);
    }

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
    fputs("\",\"type\":\"", file);
    fputs("androidNdk", file);
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
void output_error(struct bugsnag_error *er, FILE* file) {

    fputs("{ \"payloadVersion\":\"", file);
    fputs(er->payload_version, file);

    if (strlen(er->context) > 1) {
        fputs("\",\"context\":\"",file);
        fputs(er->context, file);
    }

    fputs("\",\"severity\":\"",file);
    fputs(er->severity, file);

    fputs("\",\"exceptions\":[{", file);
    output_exception(er->exception, file);
    fputs("}]", file);

    if (strlen(er->user.id) > 1 || strlen(er->user.email) > 1 || strlen(er->user.name) > 1) {
        fputs(",\"user\":{", file);
        output_user(er->user, file);
        fputs("}", file);
    }

    fputs(",\"app\": {",file);
    output_app_data(er->app_data, file);
    fputs("}", file);

    fputs(",\"device\": {",file);
    output_device(er->device, file);
    fputs("}", file);

    fputs("}", file);
}


/**
 * Gets the value from a method that returns a string
 */
const char *get_method_string(JNIEnv *env,
                              jclass class,
                              const char *method_name) {
    jmethodID method = (*env)->GetStaticMethodID(env, class, method_name, "()Ljava/lang/String;");
    jstring value = (*env)->CallStaticObjectMethod(env, class, method);

    if (value == NULL) {
        return "";
    } else {
        return (*env)->GetStringUTFChars(env, value, JNI_FALSE);
    }
}

/**
 * Gets the value from a method that contains an int
 */
int get_method_int(JNIEnv *env,
                  jclass class,
                  const char *method_name) {

    jmethodID method = (*env)->GetStaticMethodID(env, class, method_name, "()I");
    jint value = (*env)->CallStaticIntMethod(env, class, method);

    if (value == NULL) {
        return 0;
    } else {
        return (int)value;
    }
}

/**
 * Gets the value from a method that contains a java.lang.float
 */
float get_method_float(JNIEnv *env,
                      jclass class,
                      const char *method_name) {

    jmethodID method = (*env)->GetStaticMethodID(env, class, method_name, "()F");
    jfloat value = (*env)->CallStaticFloatMethod(env, class, method);

    return (float)value;
}

/**
 * Gets the value from a method that contains a java.lang.long
 */
double get_method_double(JNIEnv *env,
                      jclass class,
                      const char *method_name) {
    jmethodID method = (*env)->GetStaticMethodID(env, class, method_name, "()D");
    jdouble value = (*env)->CallStaticDoubleMethod(env, class, method);

    return (double)value;
}

/**
 * Gets the value from a method that contains a java.lang.boolean
 * returns "true" or "false" string
 */
const char* get_method_boolean(JNIEnv *env,
                              jclass class,
                              const char *method_name) {
    jmethodID method = (*env)->GetStaticMethodID(env, class, method_name, "()Ljava/lang/Boolean;");
    jobject value_boolean = (*env)->CallStaticObjectMethod(env, class, method);

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
void populate_user_details(JNIEnv *env, struct bugsnag_error *er, jclass interface_class) {
    // Get the user information
    er->user.id = get_method_string(env, interface_class, "getUserId");
    er->user.email = get_method_string(env, interface_class, "getUserEmail");
    er->user.name = get_method_string(env, interface_class, "getUserName");
}

/**
 * Gets the app data details from the client class and pre-populates the bugsnag error
 */
void populate_app_data(JNIEnv *env, struct bugsnag_error *er, jclass interface_class) {
    // Get the App Data
    er->app_data.package_name = get_method_string(env, interface_class, "getPackageName");
    er->app_data.app_name = get_method_string(env, interface_class, "getAppName");
    er->app_data.version_name = get_method_string(env, interface_class, "getVersionName");
    er->app_data.version_code = get_method_int(env, interface_class, "getVersionCode");
    er->app_data.build_uuid = get_method_string(env, interface_class, "getBuildUUID");
    er->app_data.version = get_method_string(env, interface_class, "getAppVersion");
    er->app_data.release_stage = get_method_string(env, interface_class, "getReleaseStage");
}

/**
 * Gets the device data details from the client class and pre-populates the bugsnag error
 */
void populate_device_data(JNIEnv *env, struct bugsnag_error *er, jclass interface_class) {
    // Get the device data
    er->device.os_name = "android";

    er->device.id = get_method_string(env, interface_class, "getDeviceId");
    er->device.locale = get_method_string(env, interface_class, "getDeviceLocale");
    er->device.total_memory = get_method_double(env, interface_class, "getDeviceTotalMemory");
    er->device.rooted = get_method_boolean(env, interface_class, "getDeviceRooted");
    er->device.screen_density = get_method_float(env, interface_class, "getDeviceScreenDensity");
    er->device.dpi = get_method_int(env, interface_class, "getDeviceDpi");
    er->device.screen_resolution = get_method_string(env, interface_class, "getDeviceScreenResolution");

    er->device.manufacturer = get_method_string(env, interface_class, "getDeviceManufacturer");
    er->device.brand = get_method_string(env, interface_class, "getDeviceBrand");
    er->device.model = get_method_string(env, interface_class, "getDeviceModel");
    er->device.os_version = get_method_string(env, interface_class, "getDeviceOsVersion");
    er->device.os_build = get_method_string(env, interface_class, "getDeviceOsBuild");

    er->device.api_level  = get_method_int(env, interface_class, "getDeviceApiLevel");
}

/**
 * Gets details from java to pre-populates the bugsnag error
 */
void populate_error_details(JNIEnv *env, struct bugsnag_error *er) {

    // Constant values for payload version and severity
    er->payload_version = "3";
    er->severity = "error";

    // Get the Client object
    jclass interface_class = (*env)->FindClass(env, "com/bugsnag/android/NativeInterface");

    // populate the context
    er->context = get_method_string(env, interface_class, "getContext");

    // Get the error store path
    sprintf(er->error_store_path, "%s", get_method_string(env, interface_class, "getErrorStorePath"));

    populate_user_details(env, er, interface_class);

    populate_app_data(env, er, interface_class);

    populate_device_data(env, er, interface_class);
}
