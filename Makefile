all: build

API9_DIR=tmp/api9/outputs/aar
API21_DIR=tmp/api21/outputs/aar
AAR_FILENAME=bugsnag-android-ndk-release.aar
BASEDIR=$(shell pwd)
RELEASE_ARCHIVE=tmp/bugsnag-android-ndk.aar

.PHONY: build test clean release

$(RELEASE_ARCHIVE): $(API9_DIR)/release/jni/x86 \
									 $(API9_DIR)/release/jni/armeabi \
									 $(API9_DIR)/release/jni/armeabi-v7a \
									 $(API9_DIR)/release/jni/x86_64 \
	                                 $(API9_DIR)/release/jni/arm64-v8a
	@cd $(API9_DIR)/release && zip -r $(BASEDIR)/$@ .
	@echo Generated release archive at $(RELEASE_ARCHIVE)

$(API9_DIR)/release/jni/x86: $(API9_DIR)/$(AAR_FILENAME)
	@unzip -o $< -d $(API9_DIR)/release

$(API9_DIR)/release/jni/armeabi: $(API9_DIR)/$(AAR_FILENAME)
	@unzip -o $< -d $(API9_DIR)/release

$(API9_DIR)/release/jni/armeabi_v7a: $(API9_DIR)/$(AAR_FILENAME)
	@unzip -o $< -d $(API9_DIR)/release

$(API9_DIR)/release/jni/%: $(API9_DIR)/$(AAR_FILENAME) $(API21_DIR)/release/jni/%
	@cp -r $(API21_DIR)/release/jni/$(@F) $@

$(API21_DIR)/release/jni/%: $(API21_DIR)/$(AAR_FILENAME)
	@unzip -o $< -d $(API21_DIR)/release

$(API9_DIR)/$(AAR_FILENAME):
	@./gradlew clean build -PbuildDir=tmp/api9 -PndkPVersion=9

$(API21_DIR)/$(AAR_FILENAME):
	@./gradlew clean build -PbuildDir=tmp/api21 -PndkPVersion=21

build:
	@./gradlew build

clean:
	@./gradlew clean
	@rm -rf tmp

test:
	@./gradlew :connectedCheck

release: $(RELEASE_ARCHIVE)


bump:
ifeq ($(VERSION),)
	@$(error VERSION is not defined. Run with `make VERSION=number bump`)
endif
	@echo Bumping the version number to $(VERSION)
	@sed -i '' "s/VERSION_NAME=.*/VERSION_NAME=$(VERSION)/" gradle.properties

upgrade_vendor:
ifeq ($(VERSION),)
	@$(error VERSION is not defined. Run with `make VERSION=number upgrade_vendor`)
endif
	@echo Bumping the SDK notifier number to $(VERSION)
	@sed -i '' "s/BUGSNAG_ANDROID_VERSION=.*/BUGSNAG_ANDROID_VERSION=$(VERSION)/" gradle.properties

# Makes a release
publish:
ifeq ($(VERSION),)
	@$(error VERSION is not defined. Run with `make VERSION=number publish`)
endif
	make VERSION=$(VERSION) bump && git commit -am "v$(VERSION)" && git tag v$(VERSION) \
	&& git push origin && git push --tags && ./gradlew clean assemble uploadArchives bintrayUpload
