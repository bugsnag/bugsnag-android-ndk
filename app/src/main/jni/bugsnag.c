#include <jni.h>
#include <android/log.h>
#include "bugsnag.h"

// Platform specific libraries
// TODO: check on various platforms
#include <signal.h>
#include <stdio.h>
#include <dlfcn.h>
#include <unistd.h>
#include <unwind.h>

// Globals
JNIEnv *g_env;
struct sigaction sa;

/* Signals to be caught. */
#define SIG_CATCH_COUNT 6
static const int native_sig_catch[SIG_CATCH_COUNT + 1]
        = { SIGABRT, SIGILL, SIGTRAP, SIGBUS, SIGFPE, SIGSEGV };
/* Maximum value of a caught signal. */
#define SIG_NUMBER_MAX 32

struct sigaction *sa_old;


typedef struct native_code_handler_struct {
    void *uframes[FRAMES_MAX];
} native_code_handler_struct;

static native_code_handler_struct *g_native_code;


struct BacktraceState
{
    void** current;
    void** end;
};


static _Unwind_Reason_Code unwindCallback(struct _Unwind_Context* context, void* arg)
{
    struct BacktraceState* state = (struct BacktraceState*)arg;
    uintptr_t pc = _Unwind_GetIP(context);
    if (pc) {
        if (state->current == state->end) {
            return _URC_END_OF_STACK;
        } else {
            *state->current++ = (void*)pc;
        }
    }
    return _URC_NO_REASON;
}

size_t captureBacktrace(void** buffer, size_t max)
{
    struct BacktraceState state = {buffer, buffer + max};
    _Unwind_Backtrace(unwindCallback, &state);

    return state.current - buffer;
}

/**
 * Gets a string representation of the error code
 */
char* getSignalName(int code) {
    if (code == SIGHUP) {
        return "SIGHUP";
    } else if (code == SIGINT) {
        return "SIGINT";
    } else if (code == SIGQUIT ) {
        return "SIGQUIT";
    } else if (code == SIGILL ) {
        return "SIGILL";
    } else if (code == SIGTRAP ) {
        return "SIGTRAP";
    } else if (code == SIGABRT ) {
        return "SIGABRT";
    } else if (code == SIGBUS ) {
        return "SIGBUS";
    } else if (code == SIGFPE ) {
        return "SIGFPE";
    } else if (code == SIGKILL ) {
        return "SIGKILL";
    } else if (code == SIGUSR1 ) {
        return "SIGUSR1";
    } else if (code == SIGSEGV ) {
        return "SIGSEGV";
    } else if (code == SIGUSR2 ) {
        return "SIGUSR2";
    } else if (code == SIGPIPE ) {
        return "SIGPIPE";
    } else if (code == SIGALRM ) {
        return "SIGALRM";
    } else if (code == SIGTERM ) {
        return "SIGTERM";
    } else if (code == SIGCHLD ) {
        return "SIGCHLD";
    } else if (code == SIGCONT ) {
        return "SIGCONT";
    } else if (code == SIGSTOP ) {
        return "SIGSTOP";
    } else if (code == SIGTSTP ) {
        return "SIGTSTP";
    } else if (code == SIGTTIN ) {
        return "SIGTTIN";
    } else if (code == SIGTTOU ) {
        return "SIGTTOU";
    } else if (code == SIGURG ) {
        return "SIGURG";
    } else if (code == SIGXCPU ) {
        return "SIGXCPU";
    } else if (code == SIGXFSZ ) {
        return "SIGXFSZ";
    } else if (code == SIGVTALRM ) {
        return "SIGVTALRM";
    } else if (code == SIGPROF ) {
        return "SIGPROF";
    } else if (code == SIGWINCH ) {
        return "SIGWINCH";
    } else if (code == SIGIO ) {
        return "SIGIO";
    } else if (code == SIGPWR ) {
        return "SIGPWR";
    } else if (code == SIGSYS ) {
        return "SIGSYS";
    } else if (code == __SIGRTMIN ) {
        return "__SIGRTMIN";
    } else {
        return "UNKNOWN";
    }
}

/**
 * Handles signals when errors occur and notifies Bugsnag
 */
static void signal_handler(int code, struct siginfo* si, void* sc) {
    __android_log_print(ANDROID_LOG_VERBOSE, "BugsnagNdk", "In signal_handler with signal %d", si->si_signo);

    int frames_size = (int)captureBacktrace(g_native_code->uframes, FRAMES_MAX);

    __android_log_print(ANDROID_LOG_VERBOSE, "BugsnagNdk", "after unwind frames = %d", frames_size);

    // Ignore the first 3 frames
    int project_frames = frames_size - FRAMES_TO_IGNORE;

    jclass traceClass = (*g_env)->FindClass(g_env, "java/lang/StackTraceElement");
    jmethodID traceConstructor = (*g_env)->GetMethodID(g_env, traceClass, "<init>",
                                                       "(Ljava/lang/String;Ljava/lang/String;Ljava/lang/String;I)V");
    jobjectArray trace = (*g_env)->NewObjectArray(g_env,
                                                  0,
                                                  (*g_env)->FindClass(g_env, "java/lang/StackTraceElement"),
                                                  NULL);

    if (project_frames > 0) {
        // Create stack trace array
        trace = (*g_env)->NewObjectArray(g_env,
                                         project_frames,
                                        (*g_env)->FindClass(g_env, "java/lang/StackTraceElement"),
                                        NULL);

        // populate stack frame element array
        int i;
        for (i = 0; i < project_frames; i++) {
            void *const addr = g_native_code->uframes[i + FRAMES_TO_IGNORE];

            Dl_info info;
            if (dladdr(addr, &info) != 0 && info.dli_fname != NULL) {

                // Attempt to calculate the line numbers TODO: this seems to produce incorrect results
                uintptr_t near = (uintptr_t) info.dli_saddr;
                uintptr_t offs = (uintptr_t) g_native_code->uframes[i + FRAMES_TO_IGNORE] - near;

                // Build up a stack frame
                jstring class = (*g_env)->NewStringUTF(g_env, "");
                jstring filename = (*g_env)->NewStringUTF(g_env, info.dli_fname);
                jstring methodName = (*g_env)->NewStringUTF(g_env, info.dli_sname == NULL ? "(null)"
                                                                                          : info.dli_sname);
                jint lineNumber = (int) offs;

                jobject frame = (*g_env)->NewObject(g_env,
                                                    traceClass,
                                                    traceConstructor,
                                                    class,
                                                    methodName,
                                                    filename,
                                                    lineNumber);


                (*g_env)->SetObjectArrayElement(g_env, trace, i, frame);
            }
        }
    }

    // Create an error name
    jstring name = (*g_env)->NewStringUTF(g_env, "Native error");

    // Create an exception message
    char messageBuffer[256] = { 0 };
    sprintf(messageBuffer,"Fatal signal from native: %d (%s), code %d", si->si_signo, getSignalName(si->si_signo), si->si_code);
    jstring message = (*g_env)->NewStringUTF(g_env, messageBuffer);

    // Create a severity Error
    jclass severityClass = (*g_env)->FindClass(g_env, "com/bugsnag/android/Severity");
    jfieldID errorField = (*g_env)->GetStaticFieldID(g_env, severityClass , "ERROR", "Lcom/bugsnag/android/Severity;");
    jobject severity = (*g_env)->GetStaticObjectField(g_env, severityClass, errorField);

    // Create an empty metaData object
    jclass metaDataClass = (*g_env)->FindClass(g_env, "com/bugsnag/android/MetaData");
    jmethodID metaDataConstructor = (*g_env)->GetMethodID(g_env, traceClass, "<init>", "()V");
    jobject metaData = (*g_env)->NewObject(g_env, metaDataClass, metaDataConstructor);

    // Notify Bugsnag of the error
    // TODO: currently this doesn't send anything because the app crashes before the call is made
    // This may need to be cached for sending later, or sent in a blocking thread
    jclass objclass = (*g_env)->FindClass(g_env, "com/bugsnag/android/Bugsnag");
    jmethodID method = (*g_env)->GetStaticMethodID(g_env, objclass, "notify", "(Ljava/lang/String;Ljava/lang/String;[Ljava/lang/StackTraceElement;Lcom/bugsnag/android/Severity;Lcom/bugsnag/android/MetaData;)V");
    (*g_env)->CallStaticVoidMethod(g_env, objclass, method, name, message, trace, severity, metaData);

    // Wait for the Bugsnag to be send TODO: remove this
    sleep(1);


    /* Call previous handler. */
    if (si->si_signo >= 0 && si->si_signo < SIG_NUMBER_MAX) {
        sa_old[si->si_signo].sa_sigaction(code, si, sc);
    }
}

/**
 * Adds the Bugsnag signal handler
 */
int setupBugsnag(JNIEnv *env) {
    g_env = env;

    g_native_code = calloc(sizeof(native_code_handler_struct), 1);

    // create a signal handler
    memset(&sa, 0, sizeof(sa));
    sigemptyset(&sa.sa_mask);
    sa.sa_sigaction = signal_handler;
    sa.sa_flags = SA_SIGINFO;

    sa_old = calloc(sizeof(struct sigaction), SIG_NUMBER_MAX);

    // TODO: add more signals and test them
    int i;
    for (i = 0; i < SIG_CATCH_COUNT; i++) {
        const int sig = native_sig_catch[i];
        sigaction(sig, &sa, &sa_old[sig]);
    }

    return 0;
}

/**
 * Removes the Bugsnag signal handler
 */
void tearDownBugsnag() {
    // replace signal handler with old one again
    int i;
    for(i = 0; i < SIG_CATCH_COUNT; i++) {
        int sig = native_sig_catch[i];
        sigaction(sig, &sa_old[sig], NULL);
    }

    free(&sa);
    free(&g_native_code);
    free(&sa_old);
}

