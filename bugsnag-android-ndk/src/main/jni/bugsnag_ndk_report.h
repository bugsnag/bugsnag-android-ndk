//
// Created by Dave Perryman on 04/10/2016.
//

#ifndef BUGSNAGNDK_REPORT_H
#define BUGSNAGNDK_REPORT_H

#include "deps/bugsnag/bugsnag.h"

/* Number of frames to ignore (part of handler code) */
#define BUGSNAG_FRAMES_TO_IGNORE 0

struct bugsnag_ndk_report {
    char *error_store_path;
    bugsnag_report *report;
    bsg_event *event;
    bsg_exception *exception;
};

char *bsg_load_error_store_path(JNIEnv *env);
void bsg_load_release_stages(JNIEnv *env);

void bsg_populate_event_details(JNIEnv *env, struct bugsnag_ndk_report *report);
void bsg_populate_user_details(JNIEnv *env, bsg_event *event);
void bsg_populate_app_data(JNIEnv *env,bsg_event *event);
void bsg_populate_device_data(JNIEnv *env, bsg_event *event);
void bsg_populate_context(JNIEnv *env, bsg_event *event);
void bsg_populate_breadcrumbs(JNIEnv *env, bsg_event *event);
void bsg_populate_meta_data(JNIEnv *env, bsg_event *event);

#endif //BUGSNAGNDK_REPORT_H
