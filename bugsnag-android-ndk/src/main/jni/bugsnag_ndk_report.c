#include <jni.h>

#include "bugsnag_ndk_report.h"


/**
 * Gets the value from a method that returns a string
 */
char *get_method_string(JNIEnv *env, jclass class, const char *method_name) {
    jmethodID method = (*env)->GetStaticMethodID(env, class, method_name, "()Ljava/lang/String;");
    jstring value = (*env)->CallStaticObjectMethod(env, class, method);

    if (value)
        return (char *)(*env)->GetStringUTFChars(env, value, JNI_FALSE);

    return "";
}

/**
 * Gets the value from a method that contains an int
 */
int get_method_int(JNIEnv *env, jclass class, const char *method_name) {
    jmethodID method = (*env)->GetStaticMethodID(env, class, method_name, "()I");
    jint value = (*env)->CallStaticIntMethod(env, class, method);

    if (value)
        return (int)value;

    return -1;
}

/**
 * Gets the value from a method that contains a java.lang.float
 */
float get_method_float(JNIEnv *env, jclass class, const char *method_name) {
    jmethodID method = (*env)->GetStaticMethodID(env, class, method_name, "()F");
    return (float)(*env)->CallStaticFloatMethod(env, class, method);
}

/**
 * Gets the value from a method that contains a java.lang.long
 */
double get_method_double(JNIEnv *env, jclass class, const char *method_name) {
    jmethodID method = (*env)->GetStaticMethodID(env, class, method_name, "()D");
    return (double)(*env)->CallStaticDoubleMethod(env, class, method);
}

/**
 * Gets the value from a method that contains a java.lang.boolean
 */
int get_method_boolean(JNIEnv *env, jclass class, const char *method_name) {
    jmethodID method = (*env)->GetStaticMethodID(env, class, method_name, "()Ljava/lang/Boolean;");
    jobject value_boolean = (*env)->CallStaticObjectMethod(env, class, method);

    jclass boolean_class = (*env)->FindClass(env, "java/lang/Boolean");
    jmethodID get_string_method = (*env)->GetMethodID(env, boolean_class, "toString", "()Ljava/lang/String;");
    jstring value = (*env)->CallObjectMethod(env, value_boolean, get_string_method);

    if (value) {
        return 1;
    } else {
        return 0;
    }
}

/**
 * Gets the user details from the client class and pre-populates the bugsnag error
 */
void bsg_populate_user_details(JNIEnv *env, jclass interface_class, bsg_event *event) {
    bugsnag_event_set_string(event, BSG_USER, "id", get_method_string(env, interface_class, "getUserId"));
    bugsnag_event_set_string(event, BSG_USER, "email", get_method_string(env, interface_class, "getUserEmail"));
    bugsnag_event_set_string(event, BSG_USER, "name", get_method_string(env, interface_class, "getUserName"));
}

/**
 * Gets the app data details from the client class and pre-populates the bugsnag error
 */
void bsg_populate_app_data(JNIEnv *env, jclass interface_class, bsg_event *event) {
    bugsnag_event_set_string(event, BSG_APP, "releaseStage", get_method_string(env, interface_class, "getReleaseStage"));
    bugsnag_event_set_string(event, BSG_APP, "packageName", get_method_string(env, interface_class, "getPackageName"));
    bugsnag_event_set_string(event, BSG_APP, "name", get_method_string(env, interface_class, "getAppName"));
    bugsnag_event_set_string(event, BSG_APP, "version", get_method_string(env, interface_class, "getAppVersion"));
    bugsnag_event_set_string(event, BSG_APP, "versionName", get_method_string(env, interface_class, "getVersionName"));
    bugsnag_event_set_number(event, BSG_APP, "versionCode", get_method_int(env, interface_class, "getVersionCode"));
    bugsnag_event_set_string(event, BSG_APP, "buildUUID", get_method_string(env, interface_class, "getBuildUUID"));
}

/**
 * Gets the device data details from the client class and pre-populates the bugsnag error
 */
void bsg_populate_device_data(JNIEnv *env, jclass interface_class, bsg_event *event) {
    bugsnag_event_set_string(event, BSG_DEVICE, "osName", "Android");
    bugsnag_event_set_string(event, BSG_DEVICE, "osVersion", get_method_string(env, interface_class, "getDeviceOsVersion"));
    bugsnag_event_set_string(event, BSG_DEVICE, "osBuild", get_method_string(env, interface_class, "getDeviceOsBuild"));
    bugsnag_event_set_string(event, BSG_DEVICE, "id", get_method_string(env, interface_class, "getDeviceId"));
    bugsnag_event_set_number(event, BSG_DEVICE, "totalMemory", get_method_double(env, interface_class, "getDeviceTotalMemory"));
    bugsnag_event_set_string(event, BSG_DEVICE, "locale", get_method_string(env, interface_class, "getDeviceLocale"));

    bugsnag_event_set_bool(event, BSG_DEVICE, "rooted", get_method_boolean(env, interface_class, "getDeviceRooted"));
    bugsnag_event_set_number(event, BSG_DEVICE, "dpi", get_method_int(env, interface_class, "getDeviceDpi"));
    bugsnag_event_set_number(event, BSG_DEVICE, "screenDensity", get_method_float(env, interface_class, "getDeviceScreenDensity"));
    bugsnag_event_set_string(event, BSG_DEVICE, "screenResolution", get_method_string(env, interface_class, "getDeviceScreenResolution"));

    bugsnag_event_set_string(event, BSG_DEVICE, "manufacturer", get_method_string(env, interface_class, "getDeviceManufacturer"));
    bugsnag_event_set_string(event, BSG_DEVICE, "brand", get_method_string(env, interface_class, "getDeviceBrand"));
    bugsnag_event_set_string(event, BSG_DEVICE, "model", get_method_string(env, interface_class, "getDeviceModel"));
    bugsnag_event_set_number(event, BSG_DEVICE, "apiLevel", get_method_int(env, interface_class, "getDeviceApiLevel"));
}

char *bsg_load_error_store_path(JNIEnv *env) {
    jclass interface_class = (*env)->FindClass(env, "com/bugsnag/android/NativeInterface");
    return get_method_string(env, interface_class, "getErrorStorePath");
}

/**
 * Gets details from java to pre-populates the bugsnag error
 */
void bsg_populate_event_details(JNIEnv *env, struct bugsnag_ndk_report *report) {
    jclass interface_class = (*env)->FindClass(env, "com/bugsnag/android/NativeInterface");

    bsg_event *event = report->event;
    event->severity = BSG_SEVERITY_ERR;
    event->context = get_method_string(env, interface_class, "getContext");
    bsg_populate_user_details(env, interface_class, event);
    bsg_populate_app_data(env, interface_class, event);
    bsg_populate_device_data(env, interface_class, event);
}
