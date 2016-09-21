package com.bugsnag.android.bugsnagndk;

import android.support.v7.app.AppCompatActivity;
import android.os.Bundle;
import android.view.View;
import android.widget.Button;

import com.bugsnag.android.Bugsnag;

public class MainActivity extends AppCompatActivity {

    static {
        System.loadLibrary("jni-entry-point");
    }
    public native void causeFpe();

    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        setContentView(R.layout.activity_main);

        Bugsnag.init(this);

        Button clickButton = (Button) findViewById(R.id.notifyButton);
        clickButton.setOnClickListener( new View.OnClickListener() {
            @Override
            public void onClick(View v) {
                Bugsnag.getClient().notify(new RuntimeException("test notify"));
            }
        });

        clickButton = (Button) findViewById(R.id.fpeButton);
        clickButton.setOnClickListener( new View.OnClickListener() {
            @Override
            public void onClick(View v) {
                causeFpe();
            }
        });

    }
}
