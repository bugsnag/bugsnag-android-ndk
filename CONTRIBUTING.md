# Contributing

* [Fork](https://help.github.com/articles/fork-a-repo) the
  [library on GitHub](https://github.com/bugsnag/bugsnag-android-ndk)
* Build and test your changes
* Commit and push until you are happy with your contribution
* [Make a pull request](https://help.github.com/articles/using-pull-requests)


## Building

Running `./gradlew` can automatically install both the Gradle build system
and the Android SDK.

If you already have the Android SDK installed, make sure to export the
`ANDROID_HOME` environment variable, for example:

```shell
export ANDROID_HOME=/usr/local/Cellar/android-sdk/23.0.2
```

If you don't already have the Android SDK installed, it will be automatically
installed to `~/.android-sdk`.

> Note: You'll need to make sure that the `adb`, `android` and `emulator` tools
> installed as part of the Android SDK are available in your `$PATH` before
> building.


### Building the Library

You can build new `.aar` files as follows:

```shell
./gradlew clean :build
```

Files are generated into`build/outputs/aar`.

## Running the examples app

You can build and install the example app to as follows:

```shell
./gradlew clean example:installDebug
```

This builds the latest version of the library and installs an app onto your
device/emulator.

## Releasing

TODO

