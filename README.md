# Bugsnag exception reporter for Android NDK

## This repository is DEPRECATED - [Here is the new home of bugsnag-android-ndk](https://github.com/bugsnag/bugsnag-android)

[Android NDK Crash Reporting](https://www.bugsnag.com/platforms/android/) with Bugsnag helps you detect crashes from native Android C & C++ code, so you can fix issues impacting your users. 

[![Documentation](https://img.shields.io/badge/documentation-1.1.2-blue.svg)](http://docs.bugsnag.com/platforms/android/ndk/)

## Features

* Automatically report errors from native Android C/C++ code
* Show full stacktraces with shared object symbol upload
* Report [handled errors](http://docs.bugsnag.com/platforms/android/ndk/#reporting-handled-errors)
* [Log breadcrumbs](http://docs.bugsnag.com/platforms/android/ndk/#logging-breadcrumbs) which are attached to crash reports and add insight to users' actions
* [Attach user information](http://docs.bugsnag.com/platforms/android/ndk/#identifying-users) to determine how many people are affected by a crash


## Getting started

1. [Create a Bugsnag account](https://www.bugsnag.com)
1. Complete the instructions in the [integration guide](https://docs.bugsnag.com/platforms/android/ndk) to report crashes from your app
1. Report handled errors using [`bugsnag_notify()`](https://docs.bugsnag.com/android/ndk/#reporting-handled-errors)
1. Customize your integration using the [configuration options](http://docs.bugsnag.com/platforms/android/ndk/configuration-options/)


## Support

* [Read the integration guide](http://docs.bugsnag.com/platforms/android/ndk/) or [configuration options documentation](http://docs.bugsnag.com/android/ndk/configuration-options/)
* [Search open and closed issues](https://github.com/bugsnag/bugsnag-android-ndk/issues?utf8=✓&q=is%3Aissue) for similar problems
* [Report a bug or request a feature](https://github.com/bugsnag/bugsnag-android-ndk/issues/new)


## Contributing

All contributors are welcome! For information on how to build, test
and release `bugsnag-android-ndk`, see our
[contributing guide](https://github.com/bugsnag/bugsnag-android-ndk/blob/master/CONTRIBUTING.md).


## License

The Bugsnag Android NDK library is free software released under the MIT License.
See [LICENSE.txt](https://github.com/bugsnag/bugsnag-android-ndk/blob/master/LICENSE.txt)
for details.
