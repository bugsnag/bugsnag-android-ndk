package com.bugsnag.android.ndk;

import android.util.Log;

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


    private boolean ndkInitialized = false;



    @Override
    public void update(Observable o, Object arg) {
        Log.w("Bugsnag","BugsnagObserver.update() called");

        if (ndkInitialized) {
            populateErrorDetails();
        } else {
            setupBugsnag();
            ndkInitialized = true;
        }
    }
}
