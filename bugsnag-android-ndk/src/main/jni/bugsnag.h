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

/* The number of works to look through to find the next program counter */
#define WORDS_TO_SCAN 40

#ifdef __cplusplus
extern "C" {
#endif

JNIEXPORT void JNICALL Java_com_bugsnag_android_ndk_BugsnagObserver_setupBugsnag(JNIEnv *env, jobject instance);
JNIEXPORT void JNICALL Java_com_bugsnag_android_ndk_BugsnagObserver_populateErrorDetails(JNIEnv *env, jclass type);

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
