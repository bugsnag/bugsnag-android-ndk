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
JNIEXPORT void JNICALL Java_com_bugsnag_android_ndk_BugsnagObserver_populateFilterDetails(JNIEnv *env, jclass type);
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

extern void bugsnag_notify(JNIEnv *env, char* name, char* message, bsg_severity_t severity);

extern void bugsnag_set_user(char* id, char* email, char* name);

extern void bugsnag_leave_breadcrumb(const char *name, bsg_breadcrumb_t type);

extern void bugsnag_add_string_to_tab(char *tab, char *key, char *value);

extern void bugsnag_add_number_to_tab(char *tab, char *key, double value);

extern void bugsnag_add_bool_to_tab(char *tab, char *key, int value);

#ifdef __cplusplus
}
#endif

#endif