
#include <jni.h>

JNIEXPORT void JNICALL
Java_com_bugsnag_android_example_ExampleActivity_causeNpeCrash(JNIEnv *env, jobject instance) {

    int i = 0;
    int j = 45 / i;


}
