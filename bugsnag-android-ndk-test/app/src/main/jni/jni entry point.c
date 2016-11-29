#pragma GCC optimize ("O0")

#include <jni.h>
#include <signal.h>
#include <stdlib.h>
#include <dlfcn.h>

static void __attribute__((used)) somefakefunc(void) {}

typedef struct {
    int field1;
    char *field2;
} reporter_t;

int crash_abort() {
    abort();
    return 7;
}

int crash_floating_point() {
    int i = 0;
    int j = 34 / i;

    return j;
}

int crash_null_pointer() {
    int *i = NULL;
    int j = 34 / *i;

    return j;
}

int crash_released_obj() {
    reporter_t *report = malloc(sizeof(reporter_t));
    report->field1 = 6;
    free(report);

    return report->field1;
}

int crash_undefined_inst() {
#if __i386__
    __asm__ volatile("ud2" : : :);
#elif __x86_64__
    __asm__ volatile("ud2" : : :);
#elif __arm__ && __ARM_ARCH == 6 && __thumb__
    __asm__ volatile(".word 0xde00" : : :);
#elif __arm__ && __ARM_ARCH == 6
    __asm__ volatile(".long 0xf7f8a000" : : :);
#elif __arm64__
    __asm__ volatile(".long 0xf7f8a000" : : :);
#endif
    return 42;
}

int crash_write_read_only() {
    // Write to a read-only page
    volatile char *ptr = (char *)somefakefunc;
    *ptr = 0;

    return 5;
}

int crash_trap() {
    __builtin_trap();

    return 0;
}

int crash_priv_inst() {
// execute a privileged instruction
#if __i386__
    __asm__ volatile("hlt" : : :);
#elif __x86_64__
    __asm__ volatile("hlt" : : :);
#elif __arm__ && __ARM_ARCH == 7
    __asm__ volatile(".long 0xe1400070" : : :);
#elif __arm__ && __ARM_ARCH == 6 && __thumb__
    __asm__ volatile(".long 0xf5ff8f00" : : :);
#elif __arm__ && __ARM_ARCH == 6
    __asm__ volatile(".long 0xe14ff000" : : :);
#elif __arm64__
    __asm__ volatile("tlbi alle1" : : :);
#endif
    return 5;
}

int crash_stack_overflow() {
    crash_stack_overflow();

    return 4;
}


JNIEXPORT int JNICALL
Java_com_bugsnag_android_ndk_test_MainActivity_causeFpe(JNIEnv *env, jobject instance) {
    return crash_floating_point();
}

JNIEXPORT void JNICALL
Java_com_bugsnag_android_ndk_test_MainActivity_causeNpe(JNIEnv *env, jobject instance) {
    crash_null_pointer();
}

JNIEXPORT void JNICALL
Java_com_bugsnag_android_ndk_test_MainActivity_causeBus(JNIEnv *env, jobject instance) {
    crash_write_read_only();
}

JNIEXPORT void JNICALL
Java_com_bugsnag_android_ndk_test_MainActivity_causeAbort(JNIEnv *env, jobject instance) {
    crash_abort();
}

JNIEXPORT void JNICALL
Java_com_bugsnag_android_ndk_test_MainActivity_causeTrap(JNIEnv *env, jobject instance) {
    crash_trap();
}

JNIEXPORT void JNICALL
Java_com_bugsnag_android_ndk_test_MainActivity_causeIll(JNIEnv *env, jobject instance) {
    crash_priv_inst();
}

typedef enum {
    /** An unhandled exception */
            BSG_SEVERITY_ERR,
    /** A handled exception */
            BSG_SEVERITY_WARN,
    /** Custom, notable error messages */
            BSG_SEVERITY_INFO,
} bsg_severity_t;

void internal_notify(JNIEnv *env) {
    // TODO: hack to allow call to manual notify without including the bugsnag code
    // This should be replaced with a better way to include to code
    void *libbugsnag = dlopen("libbugsnag-ndk.so", RTLD_LAZY | RTLD_LOCAL);
    void (*notify) (JNIEnv *, char*, char*, bsg_severity_t) = dlsym(libbugsnag, "notify");

    notify(env, "Test error", "This is a test notify from NDK", BSG_SEVERITY_INFO);
}

JNIEXPORT void JNICALL
Java_com_bugsnag_android_ndk_test_MainActivity_nativeNotify(JNIEnv *env, jobject instance) {
    internal_notify(env);
}