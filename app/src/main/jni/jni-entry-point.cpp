
#include <jni.h>
#include <stddef.h>
#include <android/log.h>
#include <stdlib.h>
#include <signal.h>
#include "bugsnag.h"

extern "C" {
  JNIEXPORT void JNICALL Java_com_bugsnag_android_bugsnagndk_MainActivity_causeCppFpe (JNIEnv *env, jobject instance);
  JNIEXPORT void JNICALL Java_com_bugsnag_android_bugsnagndk_MainActivity_causeCppNpe (JNIEnv *env, jobject instance);
  JNIEXPORT void JNICALL Java_com_bugsnag_android_bugsnagndk_MainActivity_causeCppBus (JNIEnv *env, jobject instance);
  JNIEXPORT void JNICALL Java_com_bugsnag_android_bugsnagndk_MainActivity_causeCppAbort (JNIEnv *env, jobject instance);
  JNIEXPORT void JNICALL Java_com_bugsnag_android_bugsnagndk_MainActivity_causeCppTrap (JNIEnv *env, jobject instance);
  JNIEXPORT void JNICALL Java_com_bugsnag_android_bugsnagndk_MainActivity_causeCppIll (JNIEnv *env, jobject instance);
}
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

int cause_bus_error() {
    raise(SIGBUS);

    // TODO: figure out how to cause this error properly
    return 0;
}

int cause_abort_error() {
    abort();

    return 0;
}

int cause_trap_error() {
    raise(SIGTRAP);

    return 0;
}


int cause_ill_error() {

    // TODO: figure out how to cause this error properly
    raise(SIGILL);
    return 5;
}


JNIEXPORT void JNICALL
Java_com_bugsnag_android_bugsnagndk_MainActivity_causeCppFpe(JNIEnv *env, jobject instance) {

    //setupBugsnag(env);

    cause_floating_point_error();

    //tearDownBugsnag();
}

JNIEXPORT void JNICALL
Java_com_bugsnag_android_bugsnagndk_MainActivity_causeCppNpe(JNIEnv *env, jobject instance) {

    //setupBugsnag(env);

    cause_null_pointer_error();

    //tearDownBugsnag();
}

JNIEXPORT void JNICALL
Java_com_bugsnag_android_bugsnagndk_MainActivity_causeCppBus(JNIEnv *env, jobject instance) {

    //setupBugsnag(env);

    cause_bus_error();

    //tearDownBugsnag();
}

JNIEXPORT void JNICALL
Java_com_bugsnag_android_bugsnagndk_MainActivity_causeCppAbort(JNIEnv *env, jobject instance) {

    //setupBugsnag(env);

    cause_abort_error();

    //tearDownBugsnag();
}

JNIEXPORT void JNICALL
Java_com_bugsnag_android_bugsnagndk_MainActivity_causeCppTrap(JNIEnv *env, jobject instance) {

    //setupBugsnag(env);

    cause_trap_error();

    //tearDownBugsnag();
}

JNIEXPORT void JNICALL
Java_com_bugsnag_android_bugsnagndk_MainActivity_causeCppIll(JNIEnv *env, jobject instance) {

    //setupBugsnag(env);

    cause_ill_error();

    //tearDownBugsnag();
}



