#include <jni.h>
#include <android/log.h>
#include "bugsnag.h"

// Platform specific libraries
// TODO: check on various platforms
#include <signal.h>
#include <stdio.h>
#include <dlfcn.h>

// Globals
JNIEnv *g_env;
struct sigaction sa;
struct sigaction sa_oldSIGFPE;
void* uframes[FRAMES_MAX];

/**
 * Gets the stack trace for the given depth
 * and puts it in the given frames pointer
 * TODO: This probably only works on Android 5+ where libunwind is included
 */
static int unwind_stack(void** frames, int max_depth) {
    void *libunwind = dlopen("libunwind.so", RTLD_LAZY | RTLD_LOCAL);
    if (libunwind != NULL) {
        int (*backtrace)(void **, int) = dlsym(libunwind, "unw_backtrace");

        if (backtrace != NULL) {
            int nb = backtrace(frames, max_depth);
            if (nb > 0) {
              return nb;
            }
        } else {
            __android_log_print(ANDROID_LOG_VERBOSE, "TestApp", "symbols not found in libunwind.so");
        }
        dlclose(libunwind);
    } else {
        __android_log_print(ANDROID_LOG_VERBOSE, "TestApp", "libunwind.so could not be loaded");
    }
    return -1;
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
    __android_log_print(ANDROID_LOG_VERBOSE, "TestApp", "In signal_handler with signal %d", si->si_signo);

    int frames_size = unwind_stack(uframes, FRAMES_MAX);

    // Ignore the first 3 frames
    int project_frames = frames_size - FRAMES_TO_IGNORE;

    jobjectArray trace = (*g_env)->NewObjectArray(g_env, project_frames, (*g_env)->FindClass(g_env, "java/lang/StackTraceElement"), NULL);
    jclass traceClass = (*g_env)->FindClass(g_env, "java/lang/StackTraceElement");
    jmethodID traceConstructor = (*g_env)->GetMethodID(g_env, traceClass, "<init>", "(Ljava/lang/String;Ljava/lang/String;Ljava/lang/String;I)V");

    // Create a stack frame element array
    int i;
    for(i = 0; i < project_frames ; i++) {
        void * const addr = uframes[i + FRAMES_TO_IGNORE];

        Dl_info info;
        if (dladdr(addr, &info) != 0 && info.dli_fname != NULL) {

            // Attempt to calculate the line numbers TODO: this seems to produce incorrect results
            uintptr_t near = (uintptr_t) info.dli_saddr;
            uintptr_t offs =  (uintptr_t) uframes[i + FRAMES_TO_IGNORE] - near;

            // Build up a stack frame
            jstring class = (*g_env)->NewStringUTF(g_env, "");
            jstring filename = (*g_env)->NewStringUTF(g_env, info.dli_fname);
            jstring methodName = (*g_env)->NewStringUTF(g_env, info.dli_sname == NULL ? "(null)" : info.dli_sname);
            jint lineNumber = (int)offs;

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

    /* Call previous handler. */
    if (si->si_signo == SIGFPE) {
        sa_oldSIGFPE.sa_sigaction(code, si, sc);
    }
}

/**
 * Adds the Bugsnag signal handler
 */
int setupBugsnag(JNIEnv *env) {
    g_env = env;

    // create a signal handler
    memset(&sa, 0, sizeof(sa));
    sigemptyset(&sa.sa_mask);
    sa.sa_sigaction = signal_handler;
    sa.sa_flags = SA_SIGINFO;

    // TODO: add more signals and test them
    sigaction(SIGFPE, &sa, &sa_oldSIGFPE);

    return 0;
}

/**
 * Removes the Bugsnag signal handler
 */
void tearDownBugsnag() {
    // replace signal handler with old one again
    sigaction(SIGFPE, &sa_oldSIGFPE, &sa);
    free(&sa);
}

