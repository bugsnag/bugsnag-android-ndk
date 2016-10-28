/**
 * Bugsnag header file, for including in C code to report exceptions to Bugsnag
 */

#ifndef BUGSNAG_NDK_H
#define BUGSNAG_NDK_H

#include <jni.h>
#include <android/log.h>

#define BUGSNAG_LOG(fmt, ...) __android_log_print(ANDROID_LOG_WARN, "BugsnagNDK", fmt, ##__VA_ARGS__)
#include "deps/bugsnag/bugsnag.h"
#include "deps/bugsnag/report.h"

#ifdef __cplusplus
extern "C" {
#endif

JNIEXPORT void JNICALL Java_com_bugsnag_android_ndk_BugsnagObserver_setupBugsnag (JNIEnv *env, jobject instance);
JNIEXPORT void JNICALL Java_com_bugsnag_android_ndk_BugsnagObserver_populateErrorDetails(JNIEnv *env, jclass type);
JNIEXPORT void JNICALL Java_com_bugsnag_android_ndk_BugsnagObserver_populateUserDetails(JNIEnv *env, jclass type);
JNIEXPORT void JNICALL Java_com_bugsnag_android_ndk_BugsnagObserver_populateAppDetails(JNIEnv *env, jclass type);
JNIEXPORT void JNICALL Java_com_bugsnag_android_ndk_BugsnagObserver_populateDeviceDetails(JNIEnv *env, jclass type);
JNIEXPORT void JNICALL Java_com_bugsnag_android_ndk_BugsnagObserver_populateContextDetails(JNIEnv *env, jclass type);
JNIEXPORT void JNICALL Java_com_bugsnag_android_ndk_BugsnagObserver_populateReleaseStagesDetails(JNIEnv *env, jclass type);
JNIEXPORT void JNICALL Java_com_bugsnag_android_ndk_BugsnagObserver_populateBreadcumbDetails(JNIEnv *env, jclass type);
JNIEXPORT void JNICALL Java_com_bugsnag_android_ndk_BugsnagObserver_populateMetaDataDetails(JNIEnv *env, jclass type);

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