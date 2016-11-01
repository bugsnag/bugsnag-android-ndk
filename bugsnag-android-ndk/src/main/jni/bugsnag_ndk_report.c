#include <jni.h>
#include <string.h>

#include "bugsnag_ndk_report.h"
#include "bugsnag_ndk.h"

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
void bsg_populate_user_details(JNIEnv *env, bsg_event *event) {
    BUGSNAG_LOG("bsg_populate_user_details");
    jclass interface_class = (*env)->FindClass(env, "com/bugsnag/android/NativeInterface");

    bugsnag_event_set_string(event, BSG_USER, "id", get_method_string(env, interface_class, "getUserId"));
    bugsnag_event_set_string(event, BSG_USER, "email", get_method_string(env, interface_class, "getUserEmail"));
    bugsnag_event_set_string(event, BSG_USER, "name", get_method_string(env, interface_class, "getUserName"));
}

/**
 * Gets the app data details from the client class and pre-populates the bugsnag error
 */
void bsg_populate_app_data(JNIEnv *env, bsg_event *event) {
    BUGSNAG_LOG("bsg_populate_app_data");
    jclass interface_class = (*env)->FindClass(env, "com/bugsnag/android/NativeInterface");

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
void bsg_populate_device_data(JNIEnv *env, bsg_event *event) {
    BUGSNAG_LOG("bsg_populate_device_data");
    jclass interface_class = (*env)->FindClass(env, "com/bugsnag/android/NativeInterface");

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


/**
 * Gets the context from the client class and pre-populates the bugsnag error
 */
void bsg_populate_context(JNIEnv *env, bsg_event *event) {
    BUGSNAG_LOG("bsg_populate_context");
    jclass interface_class = (*env)->FindClass(env, "com/bugsnag/android/NativeInterface");

    event->context = get_method_string(env, interface_class, "getContext");
}

/**
 * Gets the breadcrumbs from the client class and pre-populates the bugsnag error
 */
void bsg_populate_breadcrumbs(JNIEnv *env, bsg_event *event) {
    BUGSNAG_LOG("bsg_populate_breadcrumbs");
    jclass interface_class = (*env)->FindClass(env, "com/bugsnag/android/NativeInterface");

    // TODO
}

int string_ends_with(const char * str, const char * suffix)
{
    int str_len = strlen(str);
    int suffix_len = strlen(suffix);

    if (str_len >= suffix_len) {
        return strcmp(str + (str_len-suffix_len), suffix) == 0;
    } else {
        return 0;
    }
}

int string_starts_with(const char *str, const char *pre)
{
    size_t lenpre = strlen(pre);
    size_t lenstr = strlen(str);

    if (lenstr < lenpre) {
        return 0; // false
    } else {
        return strncmp(pre, str, lenpre) == 0;
    }
}

const char* get_class_name(JNIEnv *env, jobject object) {

    jclass object_class = (*env)->FindClass(env, "java/lang/Object");
    jmethodID get_class_method = (*env)->GetMethodID(env, object_class, "getClass", "()Ljava/lang/Class;");
    jobject class = (*env)->CallObjectMethod(env, object, get_class_method);

    jclass class_class = (*env)->FindClass(env, "java/lang/Class");
    jmethodID get_name_method = (*env)->GetMethodID(env, class_class, "getName", "()Ljava/lang/String;");
    jstring class_name = (*env)->CallObjectMethod(env, class, get_name_method);

    const char* name = (*env)->GetStringUTFChars(env, class_name, 0);

    (*env)->DeleteLocalRef(env, object_class);
    (*env)->DeleteLocalRef(env, class);
    (*env)->DeleteLocalRef(env, class_class);
    (*env)->DeleteLocalRef(env, class_name);

    return name;
}

int bsg_get_map_size(JNIEnv *env, jobject value) {
    jclass map_class = (*env)->FindClass(env, "java/util/Map");
    jmethodID size_method = (*env)->GetMethodID(env, map_class, "size", "()I");
    jint size = (*env)->CallIntMethod(env, value, size_method);

    (*env)->DeleteLocalRef(env, map_class);

    return size;
}

jarray bsg_get_map_key_array(JNIEnv *env, jobject value) {
    jclass map_class = (*env)->FindClass(env, "java/util/Map");
    jmethodID key_set_method = (*env)->GetMethodID(env, map_class, "keySet", "()Ljava/util/Set;");
    jobject key_set_value = (*env)->CallObjectMethod(env, value, key_set_method);

    jclass set_class = (*env)->FindClass(env, "java/util/Set");
    jmethodID to_array_method = (*env)->GetMethodID(env, set_class, "toArray", "()[Ljava/lang/Object;");
    jarray array = (*env)->CallObjectMethod(env, key_set_value, to_array_method);

    (*env)->DeleteLocalRef(env, map_class);
    (*env)->DeleteLocalRef(env, set_class);
    (*env)->DeleteLocalRef(env, key_set_value);

    return array;
}

jobject bsg_get_item_from_map(JNIEnv *env, jobject map, jobject key) {
    jclass map_class = (*env)->FindClass(env, "java/util/Map");
    jmethodID get_method = (*env)->GetMethodID(env, map_class, "get", "(Ljava/lang/Object;)Ljava/lang/Object;");

    (*env)->DeleteLocalRef(env, map_class);

    return (*env)->CallObjectMethod(env, map, get_method, key);
}


void bsg_add_meta_data_item(JNIEnv *env, JSON_Object* object, const char* key, jobject value);
void bsg_add_meta_data_array_item(JNIEnv *env, JSON_Array* array, jobject value);

void bsg_add_meta_data_map(JNIEnv *env, JSON_Object* base, jobject value) {

    // loop over all the items in the map and add them
    int size = bsg_get_map_size(env, value);

    if (size > 0) {

        jarray key_array_value = bsg_get_map_key_array(env, value);

        int i;
        for (i = 0; i < size; i++) {
            jobject element_key = (*env)->GetObjectArrayElement(env, key_array_value, i);
            jobject element_value = bsg_get_item_from_map(env, value, element_key);

            const char* element_key_str = (*env)->GetStringUTFChars(env, (jstring)element_key, 0);

            bsg_add_meta_data_item(env, base, element_key_str, element_value);

            (*env)->DeleteLocalRef(env, element_key);
            (*env)->DeleteLocalRef(env, element_value);
        }
    }
}

void bsg_add_meta_data_array(JNIEnv *env, JSON_Array* base, jarray value) {

    // loop over all the items in the map and add them
    int size = (*env)->GetArrayLength(env, value);

    if (size > 0) {

        const char * array_type_name = get_class_name(env, value);
        int i;

        if (strcmp(array_type_name, "[I") == 0) {
            jint* elements = (*env)->GetIntArrayElements(env, value, 0);

            for (i = 0; i < size; i++) {
                bugsnag_array_set_number(base, elements[i]);
            }
        } else if (strcmp(array_type_name, "[S") == 0) {
            jshort* elements = (*env)->GetShortArrayElements(env, value, 0);

            for (i = 0; i < size; i++) {
                bugsnag_array_set_number(base, elements[i]);
            }
        } else if (strcmp(array_type_name, "[D") == 0) {
            jdouble* elements = (*env)->GetDoubleArrayElements(env, value, 0);

            for (i = 0; i < size; i++) {
                bugsnag_array_set_number(base, elements[i]);
            }
        } else if (strcmp(array_type_name, "[F") == 0) {
            jfloat* elements = (*env)->GetFloatArrayElements(env, value, 0);

            for (i = 0; i < size; i++) {
                bugsnag_array_set_number(base, elements[i]);
            }
        } else if (strcmp(array_type_name, "[J") == 0) {
            jlong* elements = (*env)->GetLongArrayElements(env, value, 0);

            for (i = 0; i < size; i++) {
                bugsnag_array_set_number(base, elements[i]);
            }
        } else if (strcmp(array_type_name, "[B") == 0) {
            jbyte* elements = (*env)->GetByteArrayElements(env, value, 0);

            for (i = 0; i < size; i++) {
                bugsnag_array_set_number(base, elements[i]);
            }
        } else if (strcmp(array_type_name, "[Z") == 0) {
            jboolean* elements = (*env)->GetBooleanArrayElements(env, value, 0);

            for (i = 0; i < size; i++) {
                bugsnag_array_set_bool(base, elements[i]);
            }
        } else if (strcmp(array_type_name, "[C") == 0) {
            jchar* elements = (*env)->GetCharArrayElements(env, value, 0);

            for (i = 0; i < size; i++) {
                // TODO: convert to a single character string?
                bugsnag_array_set_number(base, elements[i]);
            }
        } else {

            for (i = 0; i < size; i++) {
                jobject element_value = (*env)->GetObjectArrayElement(env, value, i);

                bsg_add_meta_data_array_item(env, base, element_value);
                (*env)->DeleteLocalRef(env, element_value);
            }
        }
    }
}

const char* bsg_get_meta_data_string(JNIEnv *env, jobject value) {
    return (*env)->GetStringUTFChars(env, (jstring)value, 0);
}

jint bsg_get_meta_data_int(JNIEnv *env, jobject value) {
    jclass integer_class = (*env)->FindClass(env, "java/lang/Integer");
    jmethodID get_value_method = (*env)->GetMethodID(env, integer_class, "intValue", "()I");

    (*env)->DeleteLocalRef(env, integer_class);

    return (*env)->CallIntMethod(env, value, get_value_method);
}

jfloat bsg_get_meta_data_float(JNIEnv *env, jobject value) {
    jclass float_class = (*env)->FindClass(env, "java/lang/Float");
    jmethodID get_value_method = (*env)->GetMethodID(env, float_class, "floatValue", "()F");

    (*env)->DeleteLocalRef(env, float_class);

    return (*env)->CallFloatMethod(env, value, get_value_method);
}

jdouble bsg_get_meta_data_double(JNIEnv *env, jobject value) {
    jclass double_class = (*env)->FindClass(env, "java/lang/Double");
    jmethodID get_value_method = (*env)->GetMethodID(env, double_class, "doubleValue", "()D");

    (*env)->DeleteLocalRef(env, double_class);

    return (*env)->CallDoubleMethod(env, value, get_value_method);
}

jlong bsg_get_meta_data_long(JNIEnv *env, jobject value) {
    jclass long_class = (*env)->FindClass(env, "java/lang/Long");
    jmethodID get_value_method = (*env)->GetMethodID(env, long_class, "longValue", "()J");

    (*env)->DeleteLocalRef(env, long_class);

    return (*env)->CallLongMethod(env, value, get_value_method);
}

jchar bsg_get_meta_data_character(JNIEnv *env, jobject value) {
    jclass char_class = (*env)->FindClass(env, "java/lang/Character");
    jmethodID get_value_method = (*env)->GetMethodID(env, char_class, "charValue", "()C");

    (*env)->DeleteLocalRef(env, char_class);

    return (*env)->CallCharMethod(env, value, get_value_method);
}

jbyte bsg_get_meta_data_byte(JNIEnv *env, jobject value) {
    jclass byte_class = (*env)->FindClass(env, "java/lang/Byte");
    jmethodID get_value_method = (*env)->GetMethodID(env, byte_class, "byteValue", "()B");

    (*env)->DeleteLocalRef(env, byte_class);

    return (*env)->CallByteMethod(env, value, get_value_method);
}

jshort bsg_get_meta_data_short(JNIEnv *env, jobject value) {
    jclass short_class = (*env)->FindClass(env, "java/lang/Short");
    jmethodID get_value_method = (*env)->GetMethodID(env, short_class, "shortValue", "()S");

    (*env)->DeleteLocalRef(env, short_class);

    return (*env)->CallShortMethod(env, value, get_value_method);
}

jboolean bsg_get_meta_data_boolean(JNIEnv *env, jobject value) {
    jclass bool_class = (*env)->FindClass(env, "java/lang/Boolean");
    jmethodID get_value_method = (*env)->GetMethodID(env, bool_class, "booleanValue", "()Z");

    (*env)->DeleteLocalRef(env, bool_class);

    return (*env)->CallBooleanMethod(env, value, get_value_method);
}

/**
 * Adds the given value to the given object
 */
void bsg_add_meta_data_item(JNIEnv *env, JSON_Object* object, const char* key, jobject value) {
    const char * type_name = get_class_name(env, value);

    if (string_starts_with(type_name, "[")) {
        // Create a new section with the given key
        JSON_Array* new_array = bugsnag_object_add_array(object, key);

        bsg_add_meta_data_array(env, new_array, value);
    } else if (string_ends_with(type_name, "Map")) {
        // Create a new section with the given key
        JSON_Object* new_section = bugsnag_object_add_object(object, key);

        bsg_add_meta_data_map(env, new_section, value);
    } else if (strcmp(type_name, "java.lang.String") == 0) {
        const char* value_str = bsg_get_meta_data_string(env, value);
        bugsnag_object_set_string(object, key, value_str);
    } else if (strcmp(type_name, "java.lang.Integer") == 0) {
        jint value_int = bsg_get_meta_data_int(env, value);
        bugsnag_object_set_number(object, key, value_int);
    } else if (strcmp(type_name, "java.lang.Float") == 0) {
        jfloat value_float = bsg_get_meta_data_float(env, value);
        bugsnag_object_set_number(object, key, value_float);
    } else if (strcmp(type_name, "java.lang.Double") == 0) {
        jdouble value_double = bsg_get_meta_data_double(env, value);
        bugsnag_object_set_number(object, key, value_double);
    } else if (strcmp(type_name, "java.lang.Long") == 0) {
        jlong value_long = bsg_get_meta_data_long(env, value);
        bugsnag_object_set_number(object, key, value_long);
    } else if (strcmp(type_name, "java.lang.Character") == 0) {
        jchar value_char = bsg_get_meta_data_character(env, value);
        // TODO: convert the char to a single character string??
        bugsnag_object_set_number(object, key, value_char);
    } else if (strcmp(type_name, "java.lang.Byte") == 0) {
        jbyte value_byte = bsg_get_meta_data_byte(env, value);
        bugsnag_object_set_number(object, key, value_byte);
    } else if (strcmp(type_name, "java.lang.Short") == 0) {
        jshort value_short = bsg_get_meta_data_short(env, value);
        bugsnag_object_set_number(object, key, value_short);
    } else if (strcmp(type_name, "java.lang.Boolean") == 0) {
        jboolean value_boolean = bsg_get_meta_data_boolean(env, value);
        bugsnag_object_set_bool(object, key, value_boolean);
    } else {
        BUGSNAG_LOG("unsupported type %s", type_name);

        bugsnag_object_set_string(object, key, type_name);
    }
}

/**
 * Addes the given value to the given array
 */
void bsg_add_meta_data_array_item(JNIEnv *env, JSON_Array* array, jobject value) {
    const char * type_name = get_class_name(env, value);

    if (string_starts_with(type_name, "[")) {
        // Create a new section with the given key
        JSON_Array* new_array = bugsnag_array_add_array(array);

        bsg_add_meta_data_array(env, new_array, value);
    } else if (string_ends_with(type_name, "Map")) {
        // Create a new section with the given key
        JSON_Object* new_section = bugsnag_array_add_object(array);

        bsg_add_meta_data_map(env, new_section, value);
    } else if (strcmp(type_name, "java.lang.String") == 0) {
        const char* value_str = bsg_get_meta_data_string(env, value);
        bugsnag_array_set_string(array, value_str);
    } else if (strcmp(type_name, "java.lang.Integer") == 0) {
        jint value_int = bsg_get_meta_data_int(env, value);
        bugsnag_array_set_number(array, value_int);
    } else if (strcmp(type_name, "java.lang.Float") == 0) {
        jfloat value_float = bsg_get_meta_data_float(env, value);
        bugsnag_array_set_number(array, value_float);
    } else if (strcmp(type_name, "java.lang.Double") == 0) {
        jdouble value_double = bsg_get_meta_data_double(env, value);
        bugsnag_array_set_number(array, value_double);
    } else if (strcmp(type_name, "java.lang.Long") == 0) {
        jlong value_long = bsg_get_meta_data_long(env, value);
        bugsnag_array_set_number(array, value_long);
    } else if (strcmp(type_name, "java.lang.Character") == 0) {
        jchar value_char = bsg_get_meta_data_character(env, value);
        // TODO: convert the char to a single character string??
        bugsnag_array_set_number(array, value_char);
    } else if (strcmp(type_name, "java.lang.Byte") == 0) {
        jbyte value_byte = bsg_get_meta_data_byte(env, value);
        bugsnag_array_set_number(array, value_byte);
    } else if (strcmp(type_name, "java.lang.Short") == 0) {
        jshort value_short = bsg_get_meta_data_short(env, value);
        bugsnag_array_set_number(array, value_short);
    } else if (strcmp(type_name, "java.lang.Boolean") == 0) {
        jboolean value_boolean = bsg_get_meta_data_boolean(env, value);
        bugsnag_array_set_bool(array, value_boolean);
    } else {
        BUGSNAG_LOG("unsupported type %s", type_name);
        bugsnag_array_set_string(array, type_name);
    }
}


/**
 * Gets the meta data from the client class and pre-populates the bugsnag error
 */
void bsg_populate_meta_data(JNIEnv *env, bsg_event *event) {
    BUGSNAG_LOG("bsg_populate_meta_data");

    // wipe the existing structure
    bugsnag_event_clear_metadata_base(event);

    jclass interface_class = (*env)->FindClass(env, "com/bugsnag/android/NativeInterface");

    jmethodID get_data_method = (*env)->GetStaticMethodID(env, interface_class, "getMetaData", "()Ljava/util/Map;");
    jobject meta_data_value = (*env)->CallStaticObjectMethod(env, interface_class, get_data_method);

    int size = bsg_get_map_size(env, meta_data_value);

    if (size > 0) {
        jarray key_array_value = bsg_get_map_key_array(env, meta_data_value);

        int i;
        for (i = 0; i < size; i++) {
            jobject key = (*env)->GetObjectArrayElement(env, key_array_value, i);
            const char* tab_name = (*env)->GetStringUTFChars(env, (jstring)key, 0);

            jobject tab_value = bsg_get_item_from_map(env, meta_data_value, key);

            JSON_Object* meta_data = bugsnag_event_get_metadata_base(event);
            bsg_add_meta_data_item(env, meta_data, tab_name, tab_value);

            (*env)->DeleteLocalRef(env, key);
            (*env)->DeleteLocalRef(env, tab_value);
        }

        (*env)->DeleteLocalRef(env, key_array_value);
    }


    (*env)->DeleteLocalRef(env, interface_class);
    (*env)->DeleteLocalRef(env, meta_data_value);
}

/**
 * Gets the release stages from the client to store for later
 */
void bsg_load_release_stages(JNIEnv *env) {
    BUGSNAG_LOG("bsg_load_release_stages");
    jclass interface_class = (*env)->FindClass(env, "com/bugsnag/android/NativeInterface");

    // TODO
}

char *bsg_load_error_store_path(JNIEnv *env) {
    BUGSNAG_LOG("bsg_load_error_store_path");
    jclass interface_class = (*env)->FindClass(env, "com/bugsnag/android/NativeInterface");
    return get_method_string(env, interface_class, "getErrorStorePath");
}

/**
 * Gets details from java to pre-populates the bugsnag error
 */
void bsg_populate_event_details(JNIEnv *env, struct bugsnag_ndk_report *report) {
    BUGSNAG_LOG("bsg_populate_event_details");

    bsg_event *event = report->event;
    event->severity = BSG_SEVERITY_ERR;

    bsg_populate_context(env, event);
    bsg_populate_user_details(env, event);
    bsg_populate_app_data(env, event);
    bsg_populate_device_data(env, event);
    bsg_populate_breadcrumbs(env, event);
    bsg_populate_meta_data(env, event);

    bsg_load_release_stages(env);

}
