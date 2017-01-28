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

If you are a project maintainer, you can build and release a new version of
`bugsnag-android-ndk` as follows:

### 1. Ensure you have permission to make a release

This process is a little ridiculous...

-   Create a [Sonatype JIRA](https://issues.sonatype.org) account
-   Ask in the [Bugsnag Sonatype JIRA ticket](https://issues.sonatype.org/browse/OSSRH-5533) to become a contributor
-   Ask an existing contributor (likely Simon) to confirm in the ticket
-   Wait for Sonatype them to confirm the approval

### 2. Prepare for release

-   Test unhandled and handled exception reporting via the example application,
    ensuring both kinds of reports are sent.
-   Ensure that the native API headers (in src/main/assets/include) are in sync
    with the implementation/private headers.
-   Update the `CHANGELOG` and `README.md` with any new features
-   Update the version numbers in `gradle.properties`
-   Commit and tag the release

    ```shell
    git commit -am "v1.x.x"
    git tag v1.x.x
    git push origin master && git push --tags
    ```

### 3. Release to Maven Central

-   Create a file `~/.gradle/gradle.properties` with the following contents:

    ```ini
    # Your credentials for https://oss.sonatype.org/
    # NOTE: An equals sign (`=`) in any of these fields will break the parser
    NEXUS_USERNAME=your-nexus-username
    NEXUS_PASSWORD=your-nexus-password

    # GPG key details
    signing.keyId=your-gpg-key-id # From gpg --list-keys
    signing.password=your-gpg-key-passphrase
    signing.secretKeyRingFile=/Users/{username}/.gnupg/secring.gpg
    ```

-   Build and upload the new version

    ```shell
    ./gradlew clean build publish
    ```

-   "Promote" the release build on Maven Central

    -   Go to the [sonatype open source dashboard](https://oss.sonatype.org/index.html#stagingRepositories)
    -   Click the search box at the top right, and type “com.bugsnag”
    -   Select the com.bugsnag staging repository
    -   Click the “close” button in the toolbar, no message
    -   Click the “refresh” button
    -   Select the com.bugsnag closed repository
    -   Click the “release” button in the toolbar

### 4. Upload the .aar and headers to GitHub

-   Create a "release" from your new tag on [GitHub Releases](https://github.com/bugsnag/bugsnag-android-ndk/releases)
-   Upload the generated archive file from `build/outputs/aar/bugsnag-android-ndk-release.aar` on the "edit tag" page for this release tag

### 5. Update documentation

-    Update installation instructions in the quickstart
     guides on the website with any new content (in `_android.slim`)
-    Bump the version number in the installation instructions on
     docs.bugsnag.com/platforms/android, and add any new content


