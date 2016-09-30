/**
 * Bugsnag header file, for including in C code to report exceptions to Bugsnag
 */

#ifndef BUGSNAG_H
#define BUGSNAG_H

#include <jni.h>

/* Maximum frames in a stack trace */
#define FRAMES_MAX 16

/* Number of frames to ignore (part of handler code) */
#define FRAMES_TO_IGNORE 0

/* Signals to be caught. */
#define SIG_CATCH_COUNT 6

/* Maximum value of a caught signal. */
#define SIG_NUMBER_MAX 32

/* Structure to store unwound frames */
typedef struct native_code_handler_struct {
    void *uframes[FRAMES_MAX];
} native_code_handler_struct;

/* a Bugsnag stack frame */
struct bugsnag_stack_frame {
    const char *method;
    const char *file;
    int line_number;
    // TODO: inProject?
};

/* a Bugsnag exception */
struct bugsnag_exception {
    char error_class[256];
    char message[256];
    int frames_used;
    struct bugsnag_stack_frame stack_trace[FRAMES_MAX];
};

/* a Bugsnag user */
struct bugsnag_user {
    const char* id;
    const char* email;
    const char* name;
};

/* a Bugsnag app data */
struct bugsnag_app_data {
    const char* package_name;
    const char* app_name;
    int version_code;
    const char* version_name;
    const char* release_stage;
    const char* version;
    const char* build_uuid;
};

/* a Bugsnag device */
struct bugsnag_device {
    const char* os_name;
    const char* manufacturer;
    const char* brand;
    const char* model;
    const char* id;
    int api_level;
    const char* os_version;
    const char* os_build;

    const char* locale;

    double total_memory;
    const char* rooted;
    double screen_density;
    int dpi;
    const char* screen_resolution;
    // TODO: CPU ABI?
};

/* a Bugsnag error */
struct bugsnag_error {
    const char* payload_version;
    const char* context;
    const char* severity;
    // TODO: Meta Data
    // TODO: project packages?
    struct bugsnag_exception exception;
    struct bugsnag_user user;
    struct bugsnag_app_data app_data;
    // TODO: app state? can this be done??
    struct bugsnag_device device;
    // TODO: device state? can this be done??
    // TODO: breadcrumbs??
    // TODO: will we ever want a grouping hash?
    // TODO: threads??
    const char* error_store_path;
} bugsnag_error_struct;

/**
 * Adds the Bugsnag signal handler
 */
extern int setupBugsnag(JNIEnv*);

/**
 * Removes the Bugsnag signal handler
 */
extern void tearDownBugsnag();


#endif