#include <signal.h>
#include <stdio.h>
#include <dlfcn.h>

#include <jni.h>
#include <android/log.h>

#include "headers/libunwind.h"
#include "bugsnag_ndk.h"
#include "bugsnag_ndk_report.h"
#include "bugsnag_unwind.h"
#include "deps/bugsnag/report.h"

/* Signals to be caught. */
#define BSG_SIG_CATCH_COUNT 6

/* Maximum value of a caught signal. */
#define BSG_SIG_NUMBER_MAX 32

// Globals
/* signals to be handled */
static const int native_sig_catch[BSG_SIG_CATCH_COUNT + 1]
        = { SIGILL, SIGTRAP, SIGABRT, SIGBUS, SIGFPE, SIGSEGV };

/* the Bugsnag signal handler */
struct sigaction *g_sigaction;

/* the old signal handler array */
struct sigaction *g_sigaction_old;

/* the pre-populated Bugsnag error */
struct bugsnag_ndk_report *g_bugsnag_report;

/* structure for storing the unwound stack trace */
unwind_struct *g_native_code;

/**
 * Checks to see if the given string starts with the given prefix
 */
int startsWith(const char *pre, const char *str)
{
    size_t lenpre = strlen(pre);
    size_t lenstr = strlen(str);

    if (lenstr < lenpre) {
        return 0; // false
    } else {
        return strncmp(pre, str, lenpre) == 0;
    }
}

/**
 * Handles signals when errors occur and writes a file to the Bugsnag error cache
 */
static void bugsnag_signal_handler(int code, struct siginfo *si, void *sc) {
    BUGSNAG_LOG("In bugsnag_signal_handler with signal %d", si->si_signo);

    int frames_size = bugsnag_unwind_stack(g_native_code, BUGSNAG_FRAMES_MAX, si, sc);

    // Create an exception message and class
    bsg_exception *exception = g_bugsnag_report->exception;
    sprintf(exception->message,"Fatal signal from native code: %d (%s)", si->si_signo, strsignal(si->si_signo));
    sprintf(exception->name, "%s", strsignal(si->si_signo));

    // Ignore the first 2 frames (handler code)
    int project_frames = frames_size - BUGSNAG_FRAMES_TO_IGNORE;

    if (project_frames > 0) {

        // populate stack frame element array
        int i;
        for (i = 0; i < project_frames; i++) {

            unwind_struct_frame *unwind_frame = &g_native_code->frames[i + BUGSNAG_FRAMES_TO_IGNORE];

            Dl_info info;
            if (dladdr(unwind_frame->frame_pointer, &info) != 0) {

                bsg_stackframe frame;

                if (info.dli_fname != NULL) {
                    frame.file = info.dli_fname;
                }

                // use the method from unwind if there is one
                if (strlen(unwind_frame->method) > 1) {
                    frame.method = unwind_frame->method;
                } else {
                    frame.method = info.dli_sname;
                }

                // Attempt to calculate the line numbers TODO: this gets the position in the file in bytes
                frame.load_address = (uintptr_t)info.dli_fbase;
                frame.symbol_address = (uintptr_t)info.dli_saddr;
                frame.frame_address = (uintptr_t)unwind_frame->frame_pointer;

                uintptr_t file_offset = (uintptr_t)unwind_frame->frame_pointer - (uintptr_t)info.dli_fbase;
                frame.line_number = (int)file_offset;

                //BUGSNAG_LOG("%i, %s %s %d", i, frame.file, frame.method, frame.line_number);

                // Check if this is a system file, or handler function
                if (is_system_file(frame.file)
                    || is_system_method(frame.method)) {
                    frame.in_project = 0;
                } else {
                    frame.in_project = 1;
                }

                bugsnag_exception_add_frame(exception, frame);
            }
        }
    }

    // Create a filename for the error
    time_t now = time(NULL);
    char filename[strlen(g_bugsnag_report->error_store_path) + 20];
    sprintf(filename, "%s%ld.json", g_bugsnag_report->error_store_path, now);
    FILE* file = fopen(filename, "w+");

    if (file != NULL)
    {
        char *payload = bugsnag_serialize_event(g_bugsnag_report->event);
        fputs(payload, file);

        fflush(file);
        fclose(file);
    }

    /* Call previous handler. */
    if (si->si_signo >= 0 && si->si_signo < BSG_SIG_NUMBER_MAX) {
        if (g_sigaction_old[si->si_signo].sa_sigaction != NULL) {
            g_sigaction_old[si->si_signo].sa_sigaction(code, si, sc);
        }
    }
}


/**
 * Adds the Bugsnag signal handler
 */
int setupBugsnag(JNIEnv *env) {
    g_native_code = calloc(sizeof(unwind_struct), 1);
    memset(g_native_code, 0, sizeof(struct unwind_struct));

    bugsnag_report *report = bugsnag_report_init("");
    bsg_event *event = bugsnag_event_init();
    bsg_exception *exception = bugsnag_exception_init("","");
    char *error_store_path = bsg_load_error_store_path(env);
    bugsnag_report_add_event(report, event);
    bugsnag_event_add_exception(event, exception);

    g_bugsnag_report = malloc(sizeof(struct bugsnag_ndk_report));
    g_bugsnag_report->error_store_path = error_store_path;
    g_bugsnag_report->report = report;
    g_bugsnag_report->event = event;
    g_bugsnag_report->exception = exception;

    bsg_populate_event_details(env, g_bugsnag_report);

    // create a signal handler
    g_sigaction = calloc(sizeof(struct sigaction), 1);
    memset(g_sigaction, 0, sizeof(struct sigaction));
    sigemptyset(&g_sigaction->sa_mask);
    g_sigaction->sa_sigaction = bugsnag_signal_handler;
    g_sigaction->sa_flags = SA_SIGINFO;

    g_sigaction_old = calloc(sizeof(struct sigaction), BSG_SIG_NUMBER_MAX);
    memset(g_sigaction_old, 0, sizeof(struct sigaction) * BSG_SIG_NUMBER_MAX);
    int i;
    for (i = 0; i < BSG_SIG_CATCH_COUNT; i++) {
        const int sig = native_sig_catch[i];
        sigaction(sig, g_sigaction, &g_sigaction_old[sig]);
    }

    return 0;
}


JNIEXPORT void JNICALL
Java_com_bugsnag_android_ndk_BugsnagObserver_setupBugsnag(JNIEnv *env, jclass type) {
    setupBugsnag(env);
}

JNIEXPORT void JNICALL
Java_com_bugsnag_android_ndk_BugsnagObserver_populateErrorDetails(JNIEnv *env, jclass type) {
    bsg_populate_event_details(env, g_bugsnag_report);
}

/**
 * Removes the Bugsnag signal handler
 */
void tearDownBugsnag() {
    // replace signal handler with old one again
    int i;
    for(i = 0; i < BSG_SIG_CATCH_COUNT; i++) {
        int sig = native_sig_catch[i];
        sigaction(sig, &g_sigaction_old[sig], NULL);
    }

    free(g_sigaction);
    free(g_native_code);
    bugsnag_report_free(g_bugsnag_report->report);
    free(g_bugsnag_report);
}

