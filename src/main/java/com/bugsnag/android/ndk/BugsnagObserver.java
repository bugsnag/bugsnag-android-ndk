package com.bugsnag.android.ndk;

import com.bugsnag.android.NotifyType;

import java.util.Observable;
import java.util.Observer;

/**
 * Used to wait for changes happening in Bugsnag
 */
public class BugsnagObserver implements Observer {
    static {
        System.loadLibrary("bugsnag-ndk");
    }
    public static native void setupBugsnag();
    public static native void populateErrorDetails();
    public static native void populateUserDetails();
    public static native void populateAppDetails();
    public static native void populateDeviceDetails();
    public static native void populateContextDetails();
    public static native void populateReleaseStagesDetails();
    public static native void populateFilterDetails();
    public static native void populateBreadcumbDetails();
    public static native void populateMetaDataDetails();

    private boolean ndkInitialized = false;

    @Override
    public void update(Observable o, Object arg) {
        if (!ndkInitialized) {
            setupBugsnag();
            ndkInitialized = true;
        } else if (arg instanceof Integer) {

            switch (NotifyType.fromInt((Integer)arg)) {
                case USER:
                    populateUserDetails();
                    break;

                case APP:
                    populateAppDetails();
                    break;

                case DEVICE:
                    populateDeviceDetails();
                    break;

                case CONTEXT:
                    populateContextDetails();
                    break;

                case RELEASE_STAGES:
                    populateReleaseStagesDetails();
                    break;

                case FILTERS:
                    populateFilterDetails();
                    break;

                case BREADCRUMB:
                    populateBreadcumbDetails();
                    break;

                case META:
                    populateMetaDataDetails();
                    break;

                case ALL:
                default:
                    populateErrorDetails();
                    break;
            }
        } else {
            populateErrorDetails();
        }
    }


}
