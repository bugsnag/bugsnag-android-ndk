
#include <jni.h>
#include <stdio.h>
#include <string.h>
#include <android/log.h>
#include "bugsnag_error.h"

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
void populate_user_details(JNIEnv *env, struct bugsnag_error *er, jclass client_class, jobject client) {
    // Get the user information
    jfieldID get_user_field = (*env)->GetFieldID(env, client_class, "user", "Lcom/bugsnag/android/User;");
    jobject user = (*env)->GetObjectField(env, client, get_user_field);

    er->user.id = "";
    er->user.email = "";
    er->user.name = "";

    if (user != NULL) {
        jclass user_class = (*env)->FindClass(env, "com/bugsnag/android/User");

        er->user.id = get_method_string(env, user_class, user, "getId");
        er->user.email = get_method_string(env, user_class, user, "getEmail");
        er->user.name = get_method_string(env, user_class, user, "getName");
    }
}

/**
 * Gets the app data details from the client class and pre-populates the bugsnag error
 */
void populate_app_data(JNIEnv *env, struct bugsnag_error *er, jclass client_class, jobject client) {
    // Get the App Data
    jfieldID get_app_data_field = (*env)->GetFieldID(env, client_class, "appData", "Lcom/bugsnag/android/AppData;");
    jobject app_data = (*env)->GetObjectField(env, client, get_app_data_field);

    if (app_data != NULL) {
        jclass app_data_class = (*env)->FindClass(env, "com/bugsnag/android/AppData");

        er->app_data.package_name = get_field_string(env, app_data_class, app_data, "packageName");
        er->app_data.app_name = get_field_string(env, app_data_class, app_data, "appName");
        er->app_data.version_name = get_field_string(env, app_data_class, app_data, "versionName");
        er->app_data.version_code = get_field_int(env, app_data_class, app_data, "versionCode");
        // Build UUID comes from configuration
        er->app_data.version = get_method_string(env, app_data_class, app_data, "getAppVersion");
        er->app_data.release_stage = get_method_string(env, app_data_class, app_data, "getReleaseStage");
    }

    // Get the configuration
    jfieldID get_config_field = (*env)->GetFieldID(env, client_class, "config", "Lcom/bugsnag/android/Configuration;");
    jobject config = (*env)->GetObjectField(env, client, get_config_field);
    if (config != NULL) {
        jclass config_class = (*env)->FindClass(env, "com/bugsnag/android/Configuration");

        er->app_data.build_uuid = get_method_string(env, config_class, config, "getBuildUUID");
    }
}

/**
 * Gets the device data details from the client class and pre-populates the bugsnag error
 */
void populate_device_data(JNIEnv *env, struct bugsnag_error *er, jclass client_class, jobject client) {
    // Get the device data
    er->device.os_name = "android";

    jfieldID get_device_data_field = (*env)->GetFieldID(env, client_class, "deviceData", "Lcom/bugsnag/android/DeviceData;");
    jobject device_data = (*env)->GetObjectField(env, client, get_device_data_field);

    if (device_data != NULL) {
        jclass device_data_class = (*env)->FindClass(env, "com/bugsnag/android/DeviceData");

        er->device.id = get_field_string(env, device_data_class, device_data, "id");
        er->device.locale = get_field_string(env, device_data_class, device_data, "locale");
        er->device.total_memory = get_field_long(env, device_data_class, device_data, "totalMemory");
        er->device.rooted = get_field_boolean(env, device_data_class, device_data, "rooted");
        er->device.screen_density = get_field_float(env, device_data_class, device_data, "screenDensity");
        er->device.dpi = get_field_int(env, device_data_class, device_data, "dpi");
        er->device.screen_resolution = get_field_string(env, device_data_class, device_data, "screenResolution");
    }

    // Get the android Build class
    jclass build_class = (*env)->FindClass(env, "android/os/Build");

    // TODO: perhaps cache these values in DeviceData so they are easier to access from here
    er->device.manufacturer = get_build_property_string(env, build_class, "ro.product.manufacturer");
    er->device.brand = get_build_property_string(env, build_class, "ro.product.brand");
    er->device.model = get_build_property_string(env, build_class, "ro.product.model");
    er->device.os_build = get_build_property_string(env, build_class, "ro.build.display.id");
    er->device.os_version = get_build_property_string(env, build_class, "ro.build.version.release");

    // Get the android Build class
    jclass system_properties_class = (*env)->FindClass(env, "android/os/SystemProperties");
    jmethodID method = (*env)->GetStaticMethodID(env, system_properties_class, "getInt", "(Ljava/lang/String;I)I");
    jstring param1 = (*env)->NewStringUTF(env, "ro.build.version.sdk");
    jint param2 = 0;
    jint sdk_int = (*env)->CallStaticIntMethod(env, system_properties_class, method, param1, param2);
    er->device.api_level = sdk_int;
}

/**
 * Gets details from java to pre-populates the bugsnag error
 */
void populate_error_details(JNIEnv *env, struct bugsnag_error *er) {

    // Constant values for payload version and severity
    er->payload_version = "3";
    er->severity = "error";

    // Get the Client object
    jclass bugsnag_class = (*env)->FindClass(env, "com/bugsnag/android/Bugsnag");
    jmethodID get_client_method = (*env)->GetStaticMethodID(env, bugsnag_class, "getClient", "()Lcom/bugsnag/android/Client;");
    jobject client = (*env)->CallStaticObjectMethod(env, bugsnag_class, get_client_method);
    jclass client_class = (*env)->FindClass(env, "com/bugsnag/android/Client");

    // populate the context
    er->context = get_method_string(env, client_class, client, "getContext");

    // Get the error store path
    jfieldID error_store_field = (*env)->GetFieldID(env, client_class, "errorStore", "Lcom/bugsnag/android/ErrorStore;");
    jobject error_store = (*env)->GetObjectField(env, client, error_store_field);
    jclass error_store_class = (*env)->FindClass(env, "com/bugsnag/android/ErrorStore");
    sprintf(er->error_store_path, "%s", get_field_string(env, error_store_class, error_store, "path"));

    populate_user_details(env, er, client_class, client);

    populate_app_data(env, er, client_class, client);

    populate_device_data(env, er, client_class, client);
}