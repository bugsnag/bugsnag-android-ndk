#include "bugsnag_unwind.h"

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
 * Get the stack pointer, given a pointer to a ucontext_t context.
 **/
uintptr_t get_sp_from_ucontext(const ucontext_t *uc) {

#if (defined(__arm__))
    return uc->uc_mcontext.arm_sp;
#elif defined(__aarch64__)
    return uc->uc_mcontext.sp;
#elif (defined(__x86_64__))
    return uc->uc_mcontext.gregs[REG_RSP];
#elif (defined(__i386))
  return uc->uc_mcontext.gregs[REG_ESP];
#elif (defined (__ppc__)) || (defined (__powerpc__))
  return uc->uc_mcontext.regs->nsp;
#elif (defined(__hppa__))
  return uc->uc_mcontext.sc_iaoq[0] & ~0x3UL; //TODO: what should this be
#elif (defined(__sparc__) && defined (__arch64__))
  return uc->uc_mcontext.mc_gregs[MC_SP];
#elif (defined(__sparc__) && !defined (__arch64__))
  return uc->uc_mcontext.gregs[REG_SP];
#elif (defined(__mips__))
  return uc->uc_mcontext.gregs[29];
#else
#error "Architecture is unknown, please report me!"
#endif
}

/**
 * Checks to see if the given string starts with the given prefix
 */
int starts_with(const char *pre, const char *str)
{
    if (str == NULL) {
        return 0; // false
    }

    size_t lenpre = strlen(pre);
    size_t lenstr = strlen(str);

    if (lenstr < lenpre) {
        return 0; // false
    } else {
        return strncmp(pre, str, lenpre) == 0;
    }
}

/**
 * Checks if the given string is considered a "system" method or not
 * NOTE: some methods seem to get added to binaries automatically to catch arithmetic errors
 */
int is_system_method(const char *method) {
    if (starts_with("__aeabi_", method)) {
        return 1;
    } else {
        return 0;
    }
}

/**
 * Checks if the given string should be considered a "system" file or not
 */
int is_system_file(const char *file) {
    if (starts_with("/system/", file)
        || starts_with("libc.so", file)
        || starts_with("libdvm.so", file)
        || starts_with("libcutils.so", file)
        || starts_with("[heap]", file)) {
        return 1;
    } else {
        return 0;
    }
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
 * Checks if the given address looks like a program counter or not
 */
int is_valid_pc(void* addr) {
    Dl_info info;
    if (addr != NULL
        && dladdr(addr, &info) != 0) {

        if (!(is_system_file(info.dli_fname)
              || is_system_method(info.dli_sname))) {
            return 1;
        }
    }

    return 0;
}

/**
 * Scans through the memory looking for the next program counter
 */
int look_for_next_frame(uintptr_t current_frame_base, uintptr_t* new_frame_base, uintptr_t* found_pc) {

    int j;
    for (j = 0; j < WORDS_TO_SCAN; j++) {
        uintptr_t stack_element = (current_frame_base + (j * (sizeof(uintptr_t))));
        uintptr_t stack_value = *((uintptr_t*)stack_element);

        if (is_valid_pc((void*)stack_value)) {
            *found_pc = stack_value;
            *new_frame_base = stack_element;
            return 1;
        }
    }

    return 0;
}

/**
 * A slightly hacky method that scans through the memory under the stack pointer
 * of the given context to try and create a stack trace
 */
int unwind_frame(unwind_struct* unwind, int max_depth, void* sc) {
    ucontext_t *const uc = (ucontext_t*) sc;

    int current_output_frame = 0;

    // Check the current pc to see if it is a valid frame
    uintptr_t pc = get_pc_from_ucontext(uc);
    if (is_valid_pc((void *)pc)) {
        unwind_struct_frame *frame = &unwind->frames[current_output_frame];

        sprintf(frame->method, "");
        frame->frame_pointer = (void*)pc;
        current_output_frame++;
    }

    // Get the stack pointer for this context
    uintptr_t sp = get_sp_from_ucontext(uc);

    // loop down the stack to see if there are more valid pointers
    uintptr_t current_frame_base = sp;
    while (current_output_frame < max_depth) {

        uintptr_t new_frame_base;
        uintptr_t found_pc;

        if (look_for_next_frame(current_frame_base, &new_frame_base, &found_pc)) {
            unwind_struct_frame *frame = &unwind->frames[current_output_frame];

            sprintf(frame->method, "");
            frame->frame_pointer = (void*)found_pc;

            current_frame_base = new_frame_base + (sizeof(uintptr_t));
            current_output_frame++;

        } else {
            break;
        }
    }

    if (current_output_frame > 0) {
        return current_output_frame;
    } else {
        return unwind_basic(unwind, sc);
    }
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

    if (init_local != NULL) {
        init_local(&cursor, &uwc);

        unw_word_t ip, sp;

        int count = 0;
        do {
            unwind_struct_frame *frame = &unwind->frames[count];
            //uintptr_t offset;
            get_reg(&cursor, UNW_REG_IP, &ip);
            get_reg(&cursor, UNW_REG_SP, &sp);
            //get_proc_name(&cursor, frame->method, 1024, &offset);
            frame->frame_pointer = (void*)ip;

            // Seems to crash on Android v 5.1 when reaching the bottom of the stack
            // so quit early when there are no more offsets
            //if (offset == 0) {
            //    break;
            //}

            count++;
        } while(step(&cursor) > 0 && count < max_depth);

        return count;
    } else {
        return unwind_frame(unwind, max_depth, sc);
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
        int non_system_found = 0;
        int i;
        for (i = 0; i < size; i++) {
            unwind_struct_frame *frame = &unwind->frames[i];

            backtrace_frame_t backtrace_frame = frames[i];
            backtrace_symbol_t backtrace_symbol = symbols[i];

            if (backtrace_symbol.symbol_name != NULL) {
                sprintf(frame->method, "%s", backtrace_symbol.symbol_name);
            }

            frame->frame_pointer = (void *)backtrace_frame.absolute_pc;

            if (backtrace_symbol.map_name != NULL
                && !is_system_file(backtrace_symbol.map_name)
                && (backtrace_symbol.symbol_name == NULL || !is_system_method(backtrace_symbol.symbol_name))) {
                non_system_found = 1;
            }
        }
        free_backtrace_symbols(symbols, (size_t)size);

        if (non_system_found) {
            return size;
        } else {
            return unwind_frame(unwind, max_depth, sc);
        }
    } else {
        return unwind_frame(unwind, max_depth, sc);
    }
}

/**
 * Finds a way to unwind the stack trace
 * falls back to simply returning the top frame information
 */
int bugsnag_unwind_stack(unwind_struct* unwind, int max_depth, struct siginfo* si, void* sc) {

    int size;

    void *libunwind = dlopen("libunwind.so", RTLD_LAZY | RTLD_LOCAL);
    if (libunwind != NULL) {
        size = unwind_libunwind(libunwind, unwind, max_depth, si, sc);

        dlclose(libunwind);
    } else {
        void *libcorkscrew = dlopen("libcorkscrew.so", RTLD_LAZY | RTLD_LOCAL);
        if (libcorkscrew != NULL) {
            size = unwind_libcorkscrew(libcorkscrew, unwind, max_depth, si, sc);

            dlclose(libcorkscrew);
        } else {
            size = unwind_frame(unwind, max_depth, sc);
        }
    }

    return size;
}