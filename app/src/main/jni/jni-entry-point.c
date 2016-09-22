#include <jni.h>
#include <android/log.h>
#include <stddef.h>
#include "bugsnag.h"

int cause_floating_point_error() {
    int i = 0;
    int j = 34 / i;

    return 34;
}

int cause_null_pointer_error() {
    int *i = NULL;
    int j = 34 / *i;

    return 34;
}

int call_dangerous_function(JNIEnv* env) {
    int i = 0;
    i = i + cause_floating_point_error();

    return 42;
}

JNIEXPORT void JNICALL
Java_com_bugsnag_android_bugsnagndk_MainActivity_causeFpe(JNIEnv *env, jobject instance) {

    setupBugsnag(env);

    call_dangerous_function(env);

    tearDownBugsnag();
}

JNIEXPORT void JNICALL
Java_com_bugsnag_android_bugsnagndk_MainActivity_causeNpe(JNIEnv *env, jobject instance) {

    setupBugsnag(env);

    cause_null_pointer_error();

    tearDownBugsnag();
}