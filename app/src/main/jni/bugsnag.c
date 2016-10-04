#include <jni.h>
#include <android/log.h>
#include "bugsnag.h"
#include <signal.h>
#include <stdio.h>
#include <dlfcn.h>
#include <time.h>
#include "headers/libunwind.h"
#include "bugsnag_error.h"

/* Structure to store unwound frame */
typedef struct unwind_struct_frame {
    void *frame_pointer;
    char method[1024];
    unw_word_t offset;
} unwind_struct_frame;

/* Structure to store unwound frames */
typedef struct unwind_struct {
    unwind_struct_frame frames[FRAMES_MAX];
} unwind_struct;

/* Extracted from Android's include/corkscrew/backtrace.h */
typedef struct map_info_t map_info_t;

typedef struct {
    uintptr_t absolute_pc;
    uintptr_t stack_top;
    size_t stack_size;
} backtrace_frame_t;

typedef struct {
    uintptr_t relative_pc;
    uintptr_t relative_symbol_addr;
    char* map_name;
    char* symbol_name;
    char* demangled_name;
} backtrace_symbol_t;


// Globals
/* signals to be handled */
static const int native_sig_catch[SIG_CATCH_COUNT + 1]
        = { SIGILL, SIGTRAP, SIGABRT, SIGBUS, SIGFPE, SIGSEGV };

/* the Bugsnag signal handler */
struct sigaction *g_sigaction;

/* the old signal handler array */
struct sigaction *g_sigaction_old;

/* the pre-populated Bugsnag error */
struct bugsnag_error *g_bugsnag_error;

/* structure for storing the unwound stack trace */
unwind_struct *g_native_code;

/**
 * Get the program counter, given a pointer to a ucontext_t context.
 **/
uintptr_t get_pc_from_ucontext(const ucontext_t *uc) {
#if (defined(__arm__))
    return uc->uc_mcontext.arm_pc;
#elif defined(__aarch64__)
    return uc->uc_mcontext.pc;
#elif (defined(__x86_64__))
    return uc->uc_mcontext.gregs[REG_RIP];
#elif (defined(__i386))
  return uc->uc_mcontext.gregs[REG_EIP];
#elif (defined (__ppc__)) || (defined (__powerpc__))
  return uc->uc_mcontext.regs->nip;
#elif (defined(__hppa__))
  return uc->uc_mcontext.sc_iaoq[0] & ~0x3UL;
#elif (defined(__sparc__) && defined (__arch64__))
  return uc->uc_mcontext.mc_gregs[MC_PC];
#elif (defined(__sparc__) && !defined (__arch64__))
  return uc->uc_mcontext.gregs[REG_PC];
#elif (defined(__mips__))
  return uc->uc_mcontext.gregs[31];
#else
#error "Architecture is unknown, please report me!"
#endif
}

/**
 * Just returns the first frame in the stack as we couldn't find a library to do it
 */
int unwind_basic(unwind_struct* unwind, void* sc) {
    ucontext_t *const uc = (ucontext_t*) sc;

    // Just return a single stack frame here
    unwind->frames[0].frame_pointer = (void*)get_pc_from_ucontext(uc);
    return 1;
}

/**
 * Uses libcorkscrew to unwind the stacktrace
 * This should work on android greater than 5
 */
int unwind_libunwind(void *libunwind, unwind_struct* unwind, int max_depth, struct siginfo* si, void* sc) {
    int (*getcontext) (ucontext_t*);// = dlsym(libunwind, "_Ux86_getcontext");
    int (*init_local) (unw_cursor_t*, unw_context_t*);// = dlsym(libunwind, "_Ux86_init_local");
    int (*step) (unw_cursor_t*);// = dlsym(libunwind, "_Ux86_step");
    int (*get_reg) (unw_cursor_t *, unw_regnum_t, unw_word_t *);// = dlsym(libunwind, "_Ux86_get_reg");
    int (*get_proc_name) (unw_cursor_t *, char *, size_t, unw_word_t *);

    unw_context_t uwc;
    unw_cursor_t cursor;

    ucontext_t *const uc = (ucontext_t*) sc;

#if (defined(__arm__))
    getcontext = dlsym(libunwind, "_Uarm_getcontext");
    init_local = dlsym(libunwind, "_Uarm_init_local");
    step = dlsym(libunwind, "_Uarm_step");
    get_reg = dlsym(libunwind, "_Uarm_get_reg");
    get_proc_name = dlsym(libunwind, "_Uarm_get_proc_name");

    //ucontext_t* context = (ucontext_t*)reserved;
    unw_tdep_context_t *unw_ctx = (unw_tdep_context_t*)&uwc;
    mcontext_t* sig_ctx = &uc->uc_mcontext;
    //we need to store all the general purpose registers so that libunwind can resolve
    //    the stack correctly, so we read them from the sigcontext into the unw_context
    unw_ctx->regs[UNW_ARM_R0] = sig_ctx->arm_r0;
    unw_ctx->regs[UNW_ARM_R1] = sig_ctx->arm_r1;
    unw_ctx->regs[UNW_ARM_R2] = sig_ctx->arm_r2;
    unw_ctx->regs[UNW_ARM_R3] = sig_ctx->arm_r3;
    unw_ctx->regs[UNW_ARM_R4] = sig_ctx->arm_r4;
    unw_ctx->regs[UNW_ARM_R5] = sig_ctx->arm_r5;
    unw_ctx->regs[UNW_ARM_R6] = sig_ctx->arm_r6;
    unw_ctx->regs[UNW_ARM_R7] = sig_ctx->arm_r7;
    unw_ctx->regs[UNW_ARM_R8] = sig_ctx->arm_r8;
    unw_ctx->regs[UNW_ARM_R9] = sig_ctx->arm_r9;
    unw_ctx->regs[UNW_ARM_R10] = sig_ctx->arm_r10;
    unw_ctx->regs[UNW_ARM_R11] = sig_ctx->arm_fp;
    unw_ctx->regs[UNW_ARM_R12] = sig_ctx->arm_ip;
    unw_ctx->regs[UNW_ARM_R13] = sig_ctx->arm_sp;
    unw_ctx->regs[UNW_ARM_R14] = sig_ctx->arm_lr;
    unw_ctx->regs[UNW_ARM_R15] = sig_ctx->arm_pc;
#elif (defined(__x86_64__))
    getcontext = dlsym(libunwind, "_Ux86_64_getcontext");
        init_local = dlsym(libunwind, "_Ux86_64_init_local");
        step = dlsym(libunwind, "_Ux86_64_step");
        get_reg = dlsym(libunwind, "_Ux86_64_get_reg");
        get_proc_name = dlsym(libunwind, "_Ux86_64_get_proc_name");

        uwc = *(unw_context_t*)uc;
#elif (defined(__i386))
        getcontext = dlsym(libunwind, "_Ux86_getcontext");
        init_local = dlsym(libunwind, "_Ux86_init_local");
        step = dlsym(libunwind, "_Ux86_step");
        get_reg = dlsym(libunwind, "_Ux86_get_reg");
        get_proc_name = dlsym(libunwind, "_Ux86_get_proc_name");

        uwc = *(unw_context_t*)uc;
//#elif defined(__aarch64__)
//#elif (defined (__ppc__)) || (defined (__powerpc__))
//#elif (defined(__hppa__))
//#elif (defined(__sparc__) && defined (__arch64__))
//#elif (defined(__sparc__) && !defined (__arch64__))
//#elif (defined(__mips__))
#else
        // TODO: add more supported arches
        getcontext = NULL;
        init_local = NULL;
        step = NULL;
        get_reg = NULL;
        get_proc_name = NULL;
#endif

    __android_log_print(ANDROID_LOG_VERBOSE, "BugsnagNdk", "before init_local");
    if (init_local != NULL) {
        init_local(&cursor, &uwc);

        unw_word_t ip, sp;

        int count = 0;
        do {
            unwind_struct_frame *frame = &unwind->frames[count];
            get_reg(&cursor, UNW_REG_IP, &ip);
            get_reg(&cursor, UNW_REG_SP, &sp);
            get_proc_name(&cursor, frame->method, 1024, &frame->offset);
            frame->frame_pointer = (void*)ip;

            // Seems to crash on Android v 5.1 when reaching the bottom of the stack
            // so quit early when there are no more offsets
            if (frame->offset == 0) {
                break;
            }

            count++;
        } while(step(&cursor) > 0 && count < max_depth);

        return count;
    } else {
        return unwind_basic(unwind, sc);
    }
}

/**
 * Uses libcorkscrew to unwind the stacktrace
 * This should work on android less than 5
 */
int unwind_libcorkscrew(void *libcorkscrew, unwind_struct* unwind, int max_depth, struct siginfo* si, void* sc) {
    ssize_t (*unwind_backtrace_signal_arch)
            (siginfo_t*, void*, const map_info_t*, backtrace_frame_t*, size_t, size_t);
    map_info_t* (*acquire_my_map_info_list)();
    void (*release_my_map_info_list)(map_info_t*);
    void (*get_backtrace_symbols)(const backtrace_frame_t*, size_t, backtrace_symbol_t*);
    void (*free_backtrace_symbols)(backtrace_symbol_t*, size_t);

    unwind_backtrace_signal_arch = dlsym(libcorkscrew, "unwind_backtrace_signal_arch");
    acquire_my_map_info_list = dlsym(libcorkscrew, "acquire_my_map_info_list");
    release_my_map_info_list = dlsym(libcorkscrew, "release_my_map_info_list");
    get_backtrace_symbols = dlsym(libcorkscrew, "get_backtrace_symbols");
    free_backtrace_symbols = dlsym(libcorkscrew, "free_backtrace_symbols");


    if (unwind_backtrace_signal_arch != NULL
        && acquire_my_map_info_list != NULL
        && get_backtrace_symbols != NULL
        && release_my_map_info_list != NULL
        && free_backtrace_symbols != NULL) {

        backtrace_frame_t frames[max_depth];
        backtrace_symbol_t symbols[max_depth];

        map_info_t*const info = acquire_my_map_info_list();
        ssize_t size = unwind_backtrace_signal_arch(si, sc, info, frames, 0, (size_t)max_depth);
        release_my_map_info_list(info);

        get_backtrace_symbols(frames, (size_t)size, symbols);
        int i;
        for (i = 0; i < size; i++) {
            unwind_struct_frame *frame = &unwind->frames[i];

            backtrace_frame_t backtrace_frame = frames[i];
            backtrace_symbol_t backtrace_symbol = symbols[i];
            const uintptr_t rel = backtrace_symbol.relative_pc - backtrace_symbol.relative_symbol_addr;

            if (backtrace_symbol.symbol_name != NULL) {
                sprintf(frame->method, "%s", backtrace_symbol.symbol_name);
            }

            frame->offset = rel;
            frame->frame_pointer = (void *)backtrace_frame.absolute_pc;
        }

        free_backtrace_symbols(symbols, (size_t)size);

        return size;
    } else {
        return unwind_basic(unwind, sc);
    }
}

/**
 * Finds a way to unwind the stack trace
 * falls back to simply returning the top frame information
 */
static int unwind_stack(unwind_struct* unwind, int max_depth, struct siginfo* si, void* sc) {

    int size;

    void *libunwind = dlopen("libunwind.so", RTLD_LAZY | RTLD_LOCAL);
    if (libunwind != NULL) {
        __android_log_print(ANDROID_LOG_VERBOSE, "BugsnagNdk", "could use libunwind.so");
        size = unwind_libunwind(libunwind, unwind, max_depth, si, sc);

        dlclose(libunwind);
    } else {
        void *libcorkscrew = dlopen("libcorkscrew.so", RTLD_LAZY | RTLD_LOCAL);
        if (libcorkscrew != NULL) {
            __android_log_print(ANDROID_LOG_VERBOSE, "BugsnagNdk", "could use libcorkscrew.so");
            size = unwind_libcorkscrew(libcorkscrew, unwind, max_depth, si, sc);

            dlclose(libcorkscrew);
        } else {

            void *libbacktrace = dlopen("libbacktrace.so", RTLD_LAZY | RTLD_LOCAL);
            if (libbacktrace != NULL) {
                __android_log_print(ANDROID_LOG_VERBOSE, "BugsnagNdk", "libbacktrace is present");
                dlclose(libbacktrace);
            }

            __android_log_print(ANDROID_LOG_VERBOSE, "BugsnagNdk", "falling back to basic");
            size = unwind_basic(unwind, sc);
        }
    }

    return size;
}

/**
 * Gets a string representation of the error code
 */
char* get_signal_name(int code) {
    if (code == SIGILL ) {
        return "SIGILL";
    } else if (code == SIGTRAP ) {
        return "SIGTRAP";
    } else if (code == SIGABRT ) {
        return "SIGABRT";
    } else if (code == SIGBUS ) {
        return "SIGBUS";
    } else if (code == SIGFPE ) {
        return "SIGFPE";
    } else if (code == SIGSEGV ) {
        return "SIGSEGV";
    } else {
        return "UNKNOWN";
    }
}

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
static void signal_handler(int code, struct siginfo* si, void* sc) {
    __android_log_print(ANDROID_LOG_VERBOSE, "BugsnagNdk", "In signal_handler with signal %d", si->si_signo);

    int frames_size = unwind_stack(g_native_code, FRAMES_MAX, si, sc);

    // Create an exception message and class
    sprintf(g_bugsnag_error->exception.message,"Fatal signal from native: %d (%s), code %d", si->si_signo, get_signal_name(si->si_signo), si->si_code);
    sprintf(g_bugsnag_error->exception.error_class,"Native Error: %s", get_signal_name(si->si_signo));

    // Ignore the first 2 frames (handler code)
    int project_frames = frames_size - FRAMES_TO_IGNORE;
    int frames_used = 0;

    if (project_frames > 0) {

        // populate stack frame element array
        int i;
        for (i = 0; i < project_frames; i++) {
            unwind_struct_frame *unwind_frame = &g_native_code->frames[i + FRAMES_TO_IGNORE];

            Dl_info info;
            if (dladdr(unwind_frame->frame_pointer, &info) != 0 && info.dli_fname != NULL) {

                // Check that this isn't a system file
                if (!startsWith("/system/", info.dli_fname)) {
                    struct bugsnag_stack_frame* bugsnag_frame = &g_bugsnag_error->exception.stack_trace[frames_used];

                    bugsnag_frame->file = info.dli_fname;

                    // use the method from unwind if there is one
                    if (strlen(unwind_frame->method) > 1) {
                        bugsnag_frame->method = unwind_frame->method;
                    } else {
                        bugsnag_frame->method = info.dli_sname;
                    }

                    // use the offset from unwind if there is one
                    if (unwind_frame->offset != 0) {
                        bugsnag_frame->line_number = (int) unwind_frame->offset;
                    } else {
                        // Attempt to calculate the line numbers TODO: this seems to produce slightly incorrect results
                        uintptr_t near = (uintptr_t) info.dli_saddr;
                        uintptr_t offs = (uintptr_t) unwind_frame->frame_pointer - near;

                        bugsnag_frame->line_number = (int)offs;
                    }

                    //__android_log_print(ANDROID_LOG_VERBOSE, "BugsnagNdk", "%s %s %d", bugsnag_frame->file, bugsnag_frame->method, bugsnag_frame->line_number);

                    frames_used++;
                }
            }
        }
    }
    g_bugsnag_error->exception.frames_used = frames_used;

    // Create a filename for the error
    time_t now = time(NULL);
    char filename[strlen(g_bugsnag_error->error_store_path) + 20];
    sprintf(filename, "%s%ld.json", g_bugsnag_error->error_store_path, now);
    FILE* file = fopen(filename, "w+");

    if (file != NULL)
    {
        output_error(g_bugsnag_error, file);

        fflush(file);
        fclose(file);
    }

    /* Call previous handler. */
    if (si->si_signo >= 0 && si->si_signo < SIG_NUMBER_MAX) {
        g_sigaction_old[si->si_signo].sa_sigaction(code, si, sc);
    }
}


/**
 * Adds the Bugsnag signal handler
 */
int setupBugsnag(JNIEnv *env) {
    g_native_code = calloc(sizeof(unwind_struct), 1);
    memset(g_native_code, 0, sizeof(struct unwind_struct));

    g_bugsnag_error = calloc(sizeof(bugsnag_error_struct), 1);

    populate_error_details(env, g_bugsnag_error);

    // create a signal handler
    g_sigaction = calloc(sizeof(struct sigaction), 1);
    memset(g_sigaction, 0, sizeof(struct sigaction));
    sigemptyset(&g_sigaction->sa_mask);
    g_sigaction->sa_sigaction = signal_handler;
    g_sigaction->sa_flags = SA_SIGINFO;

    g_sigaction_old = calloc(sizeof(struct sigaction), SIG_NUMBER_MAX);
    int i;
    for (i = 0; i < SIG_CATCH_COUNT; i++) {
        const int sig = native_sig_catch[i];
        sigaction(sig, g_sigaction, &g_sigaction_old[sig]);
    }

    return 0;
}


JNIEXPORT void JNICALL Java_com_bugsnag_android_bugsnagndk_MainActivity_setupBugsnag (JNIEnv *env, jobject instance) {
    setupBugsnag(env);
}

/**
 * Removes the Bugsnag signal handler
 */
void tearDownBugsnag() {
    // replace signal handler with old one again
    int i;
    for(i = 0; i < SIG_CATCH_COUNT; i++) {
        int sig = native_sig_catch[i];
        sigaction(sig, &g_sigaction_old[sig], NULL);
    }

    free(g_sigaction);
    free(g_native_code);
    free(g_bugsnag_error);
    free(g_sigaction_old);
}

