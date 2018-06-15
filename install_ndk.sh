#!/usr/bin/env bash

export OS_VERSION='darwin-x86_64'
export NDK_VERSION='r16b'

echo "Downloading NDK, this will take a while..."
wget -q https://dl.google.com/android/repository/android-ndk-${NDK_VERSION}-${OS_VERSION}.zip

echo "Unzipping NDK, this will take a while..."
unzip -q android-ndk-${NDK_VERSION}-${OS_VERSION}.zip

echo "Copying to ndk-bundle"
mv android-ndk-${NDK_VERSION} $ANDROID_HOME/ndk-bundle

export ANDROID_NDK_HOME=$ANDROID_HOME/ndk-bundle
export ANDROID_NDK=$ANDROID_HOME/ndk-bundle

# add to PATH
export PATH=${PATH}:${ANDROID_NDK_HOME}
