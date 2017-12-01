# Sets the minimum version of CMake required to build your native library.
# This ensures that a certain set of CMake features is available to
# your build.

cmake_minimum_required(VERSION 3.4.1)

# Specifies a library name, specifies whether the library is STATIC or
# SHARED, and provides relative paths to the source code. You can
# define multiple libraries by adding multiple add.library() commands,
# and CMake builds them for you. When you build your app, Gradle
# automatically packages shared libraries with your APK.

add_library( # Specifies the name of the library.
             libbugsnag-ndk

             # Sets the library as a shared library.
             SHARED

             # Provides a relative path to your source file(s).
             src/main/jni/bugsnag_ndk.h
             src/main/jni/bugsnag_ndk.c
             src/main/jni/bugsnag_ndk_report.h
             src/main/jni/bugsnag_ndk_report.c
             src/main/jni/bugsnag_unwind.h
             src/main/jni/bugsnag_unwind.c
             src/main/jni/deps/bugsnag/bugsnag.h
             src/main/jni/deps/bugsnag/report.h
             src/main/jni/deps/bugsnag/report.c
             src/main/jni/deps/bugsnag/serialize.h
             src/main/jni/deps/bugsnag/serialize.c
             src/main/jni/deps/deps/parson/parson.h
             src/main/jni/deps/deps/parson/parson.c
             )

set(BUGSNAG_VERSION 1.0.1)
target_include_directories(libbugsnag-ndk PRIVATE ${BUGSNAG_DIR}/assets/include)
