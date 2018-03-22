all: build

.PHONY: build test clean upgrade_vendor publish

build:
	@./gradlew build

clean:
	@./gradlew clean

test:
	@./gradlew :connectedCheck

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
	@make VERSION=$(VERSION) bump
	@git commit -am "Release v$(VERSION)"
	@git tag v$(VERSION)
	@git push origin master v$(VERSION)
	@./gradlew clean assemble uploadArchives bintrayUpload -Preleasing=true
