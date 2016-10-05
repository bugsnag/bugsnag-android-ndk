/**
 * Bugsnag header file, for including in C code to report exceptions to Bugsnag
 */

#ifndef BUGSNAG_H
#define BUGSNAG_H

#include <jni.h>

/* Signals to be caught. */
#define SIG_CATCH_COUNT 6

/* Maximum value of a caught signal. */
#define SIG_NUMBER_MAX 32

#ifdef __cplusplus
extern "C" {
#endif

JNIEXPORT void JNICALL Java_com_bugsnag_android_Client_setupBugsnag(JNIEnv *env, jobject instance);

/**
 * Adds the Bugsnag signal handler
 */
extern int setupBugsnag(JNIEnv *);

/**
 * Removes the Bugsnag signal handler
 */
extern void tearDownBugsnag();

#ifdef __cplusplus
}
#endif

#endif
