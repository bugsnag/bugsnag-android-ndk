## 1.2.0 (2018-06-18)

This release alters the behaviour of the notifier to track sessions automatically. 
  A session will be automatically captured on each app launch and sent to [https://sessions.bugsnag.com](https://sessions.bugsnag.com). If you
  use Bugsnag On-Premise, it is now also recommended that you set your notify and session endpoints
  via `config.setEndpoints(String notify, String sessions)`.
  
* Update bugsnag-android dependency version to v4.5.0:

  * Enable automatic session tracking by default [#314](https://github.com/bugsnag/bugsnag-android/pull/314)

  * Build bugsnag-android-ndk project with r16b [#20](https://github.com/bugsnag/bugsnag-android-ndk/pull/20)

## 1.1.3 (2018-04-16)

### Bug fixes

* Add nullcheck for reading breadcrumb metadata values [#9](https://github.com/bugsnag/bugsnag-android-ndk/pull/9)
[Jamie Lynch](https://github.com/fractalwrench)

* Update bugsnag-android dependency version to v4.3.1:

  * Fix possible ANR when enabling session tracking via
  `Bugsnag.setAutoCaptureSessions()` and connecting to latent networks.
  [#231](https://github.com/bugsnag/bugsnag-android/pull/231)

  * Fix invalid payloads being sent when processing multiple Bugsnag events in the
  same millisecond
  [#235](https://github.com/bugsnag/bugsnag-android/pull/235)

  * Re-add API key to error report HTTP request body to preserve backwards
  compatibility with older versions of the error reporting API
  [#228](https://github.com/bugsnag/bugsnag-android/pull/228)



## 1.1.2 (2018-01-19)

### Bug fixes

* Re-add armeabi support
  [#5](https://github.com/bugsnag/bugsnag-android-ndk/issues/5)
* Update bugsnag-android dependency version to v4.3.0:
  * Move capture of thread stacktraces to start of notify process
  * Add configuration option to disable automatic breadcrumb capture
  * Parse manifest meta-data for Session Auto-Capture boolean flag

## 1.1.1 (2018-01-18)

* Move capture of thread stacktraces to start of notify process
* Add configuration option to disable automatic breadcrumb capture
* Update Gradle Wrapper
* Parse manifest meta-data for Session Auto-Capture boolean flag

## 1.1.0 (2018-01-10)

* Add session tracking and update Android library

## 1.0.1 (2017-01-17)

### Bug Fixes

* Update POM to include packaging type

## 1.0.0 (2017-01-27)

Initial release
