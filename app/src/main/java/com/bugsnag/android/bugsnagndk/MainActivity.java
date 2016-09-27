package com.bugsnag.android.bugsnagndk;

import android.support.v7.app.AppCompatActivity;
import android.os.Bundle;
import android.view.View;
import android.widget.Button;

import com.bugsnag.android.Bugsnag;
import com.bugsnag.android.Configuration;

import java.util.logging.Level;
import java.util.logging.Logger;

public class MainActivity extends AppCompatActivity {

    static {
        System.loadLibrary("jni-entry-point");
    }
    public native void causeFpe();
    public native void causeNpe();

    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        setContentView(R.layout.activity_main);

        Configuration config = new Configuration("c8f000122272e15e5f06e31d540cc79c");
        config.setEndpoint("http://10.0.2.2:8000");

        Bugsnag.init(this, config);
        Bugsnag.setUser("12345", "test@example.com", "Mr Example");

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

        clickButton = (Button) findViewById(R.id.npeButton);
        clickButton.setOnClickListener( new View.OnClickListener() {
            @Override
            public void onClick(View v) {
                causeNpe();
            }
        });

    }
}
