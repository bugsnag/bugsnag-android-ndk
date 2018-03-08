package com.bugsnag.android;

import android.support.test.annotation.UiThreadTest;
import android.support.test.filters.LargeTest;
import android.support.test.rule.ActivityTestRule;
import android.support.test.runner.AndroidJUnit4;

import com.bugsnag.android.ndk.test.MainActivity;

import org.junit.Rule;
import org.junit.Test;
import org.junit.runner.RunWith;

import static junit.framework.Assert.assertNotNull;

@RunWith(AndroidJUnit4.class)
@LargeTest
public class ExampleTest {

    @Rule
    public ActivityTestRule<MainActivity> activityTestRule = new ActivityTestRule<>(MainActivity.class);

    @Test
    @UiThreadTest
    public void checkAppLaunches() throws Exception {
        // send report via activity
        final MainActivity activity = activityTestRule.getActivity();
        assertNotNull(activity);
        activity.leaveBreadcrumb();
    }

}
