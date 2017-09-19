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
             jni-entry-point

             # Sets the library as a shared library.
             SHARED

             # Provides a relative path to your source file(s).
             "src/main/jni/jni entry point.c"
             src/main/jni/jni-entry-point.cpp)

add_library(lib_bugsnag SHARED IMPORTED)
set(BUGSNAG_VERSION 1.0.1)
set(BUGSNAG_DIR ${CMAKE_SOURCE_DIR}/build/intermediates/exploded-aar/com.bugsnag/bugsnag-android-ndk/${BUGSNAG_VERSION})
set_target_properties(lib_bugsnag PROPERTIES
    IMPORTED_LOCATION ${BUGSNAG_DIR}/jni/${ANDROID_ABI}/libbugsnag-ndk.so)
target_include_directories(jni-entry-point PRIVATE ${BUGSNAG_DIR}/assets/include)
target_link_libraries(jni-entry-point lib_bugsnag)
