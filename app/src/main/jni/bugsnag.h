/**
 * Bugsnag header file, for including in C code to report exceptions to Bugsnag
 */

#ifndef BUGSNAG_H
#define BUGSNAG_H

#include <jni.h>

#define FRAMES_MAX 32
#define FRAMES_TO_IGNORE 3

/**
 * Adds the Bugsnag signal handler
 */
extern int setupBugsnag(JNIEnv*);

/**
 * Removes the Bugsnag signal handler
 */
extern void tearDownBugsnag();


#endif