
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
    //raise(SIGBUS);

    int *iptr;
    char *cptr;

#if defined(__GNUC__)
# if defined(__i386__)
    /* Enable Alignment Checking on x86 */
    __asm__("pushf\norl $0x40000,(%esp)\npopf");
# elif defined(__x86_64__)
    /* Enable Alignment Checking on x86_64 */
    __asm__("pushf\norl $0x40000,(%rsp)\npopf");
# endif
#endif

    /* malloc() always provides aligned memory */
    cptr = (char*)malloc(sizeof(int) + 1);

    /* Increment the pointer by one, making it misaligned */
    iptr = (int *) ++cptr;

    /* Dereference it as an int pointer, causing an unaligned access */
    *iptr = 42;

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

    setupBugsnag(env);

    cause_floating_point_error();

    tearDownBugsnag();
}

JNIEXPORT void JNICALL
Java_com_bugsnag_android_bugsnagndk_MainActivity_causeCppNpe(JNIEnv *env, jobject instance) {

    setupBugsnag(env);

    cause_null_pointer_error();

    tearDownBugsnag();
}

JNIEXPORT void JNICALL
Java_com_bugsnag_android_bugsnagndk_MainActivity_causeCppBus(JNIEnv *env, jobject instance) {

    setupBugsnag(env);

    cause_bus_error();

    tearDownBugsnag();
}

JNIEXPORT void JNICALL
Java_com_bugsnag_android_bugsnagndk_MainActivity_causeCppAbort(JNIEnv *env, jobject instance) {

    setupBugsnag(env);

    cause_abort_error();

    tearDownBugsnag();
}

JNIEXPORT void JNICALL
Java_com_bugsnag_android_bugsnagndk_MainActivity_causeCppTrap(JNIEnv *env, jobject instance) {

    setupBugsnag(env);

    cause_trap_error();

    tearDownBugsnag();
}

JNIEXPORT void JNICALL
Java_com_bugsnag_android_bugsnagndk_MainActivity_causeCppIll(JNIEnv *env, jobject instance) {

    setupBugsnag(env);

    cause_ill_error();

    tearDownBugsnag();
}



