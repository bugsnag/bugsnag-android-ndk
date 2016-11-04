# Add project specific ProGuard rules here.
# By default, the flags in this file are appended to flags specified
# in /Users/dave/Library/Android/sdk/tools/proguard/proguard-android.txt
# You can edit the include path and order by changing the proguardFiles
# directive in build.gradle.
#
# For more details, see
#   http://developer.android.com/guide/developing/tools/proguard.html

# Add any project specific keep options here:

# If your project uses WebView with JS, uncomment the following
# and specify the fully qualified class name to the JavaScript interface
# class:
#-keepclassmembers class fqcn.of.javascript.interface.for.webview {
#   public *;
#}

-keep class com.bugsnag.android.NativeInterface { *; }
-keep class com.bugsnag.android.Breadcrumbs { *; }
-keep class com.bugsnag.android.Breadcrumbs$Breadcrumb { *; }
-keep class com.bugsnag.android.BreadcrumbType { *; }
-keep class com.bugsnag.android.ndk.BugsnagObserver { *; }
-keep class com.bugsnag.android.ndk.test.MainActivity {
    public native void causeFpe();
    public native void causeNpe();
    public native void causeBus();
    public native void causeAbort();
    public native void causeTrap();
    public native void causeIll();
    public native void causeCppFpe();
    public native void causeCppNpe();
    public native void causeCppBus();
    public native void causeCppAbort();
    public native void causeCppTrap();
    public native void causeCppIll();
}

