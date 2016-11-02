package com.bugsnag.android.ndk.test;

import android.app.Activity;
import android.os.Bundle;
import android.view.View;
import android.widget.Button;

import com.bugsnag.android.BreadcrumbType;
import com.bugsnag.android.Bugsnag;
import com.bugsnag.android.Configuration;
import com.bugsnag.android.ndk.BugsnagObserver;

import java.util.Arrays;
import java.util.HashMap;
import java.util.List;
import java.util.Map;


public class MainActivity extends Activity {
    static {
        System.loadLibrary("jni-entry-point");
    }
    public native void causeFpe();
    public native void causeNpe();
    public native void causeBus();
    public native void causeAbort();
    public native void causeTrap();
    public native void causeIll();
    public native void causeCppFpe();
    public native void causeCppNpe();
    public native void causeCppBus();
    public native void causeCppAbort();
    public native void causeCppTrap();
    public native void causeCppIll();


    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        setContentView(R.layout.activity_main);

        BugsnagObserver ob = new BugsnagObserver();

        Configuration config = new Configuration("c8f000122272e15e5f06e31d540cc79c");
        config.setEndpoint("http://10.0.2.2:8000");//"http://notify.bugsnag.com");

        Bugsnag.init(this, config);

        Bugsnag.setFilters("KeyString","intArrayKey","submapKey");

        Bugsnag.getMetaData().addToTab("tab1", "KeyString", "StringValue1");
        Bugsnag.getMetaData().addToTab("tab1", "KeyShort", new Short("12"));
        Bugsnag.getMetaData().addToTab("tab1", "KeyInt", 123);
        Bugsnag.getMetaData().addToTab("tab1", "KeyDouble", 123.4);
        Bugsnag.getMetaData().addToTab("tab1", "KeyFloat", 123.45F);
        Bugsnag.getMetaData().addToTab("tab1", "KeyLong", 1234L);
        Bugsnag.getMetaData().addToTab("tab1", "KeyByte", new Byte("3"));
        Bugsnag.getMetaData().addToTab("tab1", "KeyBool", false);
        Bugsnag.getMetaData().addToTab("tab1", "KeyChar", new Character('c'));

        Map<String, Object> mapValue = new HashMap<>();
        mapValue.put("KeyString1", "StringValue1");
        mapValue.put("KeyInt1", 12345);
        mapValue.put("KeyIntArray", new int[] {4,6,1});
        mapValue.put("KeyObjArray", new Object[] {"StringValue3", 12346F, true});

        Map<String, Object> submapValue = new HashMap<>();
        submapValue.put("KeyString2", "StringValue2");
        submapValue.put("KeyInt2", 123456);
        mapValue.put("submapKey", submapValue);

        Bugsnag.getMetaData().addToTab("tab1", "mapKey", mapValue);


        Bugsnag.getMetaData().addToTab("tab1", "shortArrayKey", new short[] {1,2,1});
        Bugsnag.getMetaData().addToTab("tab1", "intArrayKey", new int[] {4,6,1});
        Bugsnag.getMetaData().addToTab("tab1", "doubleArrayKey", new double[] {4.4,6,1});
        Bugsnag.getMetaData().addToTab("tab1", "floatArrayKey", new float[] {4.5F,6,1});
        Bugsnag.getMetaData().addToTab("tab1", "longArrayKey", new long[] {4,6,1});
        Bugsnag.getMetaData().addToTab("tab1", "byteArrayKey", new byte[] {4,6,1});
        Bugsnag.getMetaData().addToTab("tab1", "boolArrayKey", new boolean[] {false,true,false});
        Bugsnag.getMetaData().addToTab("tab1", "charArrayKey", new char[] {'a','b','c'});

        Bugsnag.getMetaData().addToTab("tab1", "shortObjArrayKey", new Short[] {1,2,1});
        Bugsnag.getMetaData().addToTab("tab1", "intObjArrayKey", new Integer[] {4,6,1});
        Bugsnag.getMetaData().addToTab("tab1", "doubleObjArrayKey", new Double[] {4.4,6.4,1.3});
        Bugsnag.getMetaData().addToTab("tab1", "floatObjArrayKey", new Float[] {4.5F,6.0F,1.0F});
        Bugsnag.getMetaData().addToTab("tab1", "longObjArrayKey", new Long[] {4L,6L,1L});
        Bugsnag.getMetaData().addToTab("tab1", "byteObjArrayKey", new Byte[] {4,6,1});
        Bugsnag.getMetaData().addToTab("tab1", "boolObjArrayKey", new Boolean[] {false,true,false});
        Bugsnag.getMetaData().addToTab("tab1", "charObjArrayKey", new Character[] {'a','b','c'});

        Bugsnag.getMetaData().addToTab("tab1", "shortObjListKey", Arrays.asList((short)1,(short)2,(short)1));
        Bugsnag.getMetaData().addToTab("tab1", "intObjListKey", Arrays.asList(4,6,1));
        Bugsnag.getMetaData().addToTab("tab1", "doubleObjListKey", Arrays.asList(4.4,6.4,1.3));
        Bugsnag.getMetaData().addToTab("tab1", "floatObjListKey", Arrays.asList(4.5F,6.0F,1.0F));
        Bugsnag.getMetaData().addToTab("tab1", "longObjListKey", Arrays.asList(4L,6L,1L));
        Bugsnag.getMetaData().addToTab("tab1", "byteObjListKey", Arrays.asList((byte)4,(byte)6,(byte)1));
        Bugsnag.getMetaData().addToTab("tab1", "boolObjListKey", Arrays.asList(false,true,false));
        Bugsnag.getMetaData().addToTab("tab1", "charObjListKey", Arrays.asList('a','b','c'));
        Bugsnag.getMetaData().addToTab("tab1", "ObjectListKey", Arrays.asList("StringValue3", 12346F, true));


        Bugsnag.getMetaData().addToTab("tab1", "stringArrayKey", new String[] {"string1", "string2", "string3"});
        Bugsnag.getMetaData().addToTab("tab1", "objArrayKey", new Object[] {"StringValue3", 12346F, true});

        Map<String, Object> submapValue2 = new HashMap<>();
        submapValue2.put("KeyString2", "StringValue2");
        submapValue2.put("KeyInt2", 123456);
        Bugsnag.getMetaData().addToTab("tab1", "objSubArrayKey", new Object[] {new int[] {4,6,1}, new Object[] {"StringValue3", 12346F, true}, submapValue2});

        Bugsnag.getMetaData().addToTab("tab2", "Key1", "StringValue2");
        Bugsnag.getMetaData().addToTab("tab2", "Key2", 345);



        Map<String, String> values = new HashMap<>();
        values.put("KeyString1", "StringValue1");
        values.put("KeyString2", "StringValue2");
        Bugsnag.leaveBreadcrumb("Something happened here", BreadcrumbType.MANUAL, values);

        Map<String, String> values2 = new HashMap<>();
        values2.put("KeyString3", "StringValue3");
        values2.put("KeyString4", "StringValue4");
        Bugsnag.leaveBreadcrumb("Something else happened here", BreadcrumbType.MANUAL, values2);


        Bugsnag.setNotifyReleaseStages("release", "dev");



        // Set the user information
        Bugsnag.setUser("123456", "james@example.com", "James Smith");

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

        clickButton = (Button) findViewById(R.id.busButton);
        clickButton.setOnClickListener( new View.OnClickListener() {
            @Override
            public void onClick(View v) {
                causeBus();
            }
        });

        clickButton = (Button) findViewById(R.id.abortButton);
        clickButton.setOnClickListener( new View.OnClickListener() {
            @Override
            public void onClick(View v) {
                causeAbort();
            }
        });

        clickButton = (Button) findViewById(R.id.trapButton);
        clickButton.setOnClickListener( new View.OnClickListener() {
            @Override
            public void onClick(View v) {
                causeTrap();
            }
        });

        clickButton = (Button) findViewById(R.id.illButton);
        clickButton.setOnClickListener( new View.OnClickListener() {
            @Override
            public void onClick(View v) {
                causeIll();
            }
        });


        clickButton = (Button) findViewById(R.id.cppFpeButton);
        clickButton.setOnClickListener( new View.OnClickListener() {
            @Override
            public void onClick(View v) {
                causeCppFpe();
            }
        });

        clickButton = (Button) findViewById(R.id.cppNpeButton);
        clickButton.setOnClickListener( new View.OnClickListener() {
            @Override
            public void onClick(View v) {
                causeCppNpe();
            }
        });

        clickButton = (Button) findViewById(R.id.cppBusButton);
        clickButton.setOnClickListener( new View.OnClickListener() {
            @Override
            public void onClick(View v) {
                causeCppBus();
            }
        });

        clickButton = (Button) findViewById(R.id.cppAbortButton);
        clickButton.setOnClickListener( new View.OnClickListener() {
            @Override
            public void onClick(View v) {
                causeCppAbort();
            }
        });

        clickButton = (Button) findViewById(R.id.cppTrapButton);
        clickButton.setOnClickListener( new View.OnClickListener() {
            @Override
            public void onClick(View v) {
                causeCppTrap();
            }
        });

        clickButton = (Button) findViewById(R.id.cppIllButton);
        clickButton.setOnClickListener( new View.OnClickListener() {
            @Override
            public void onClick(View v) {
                causeCppIll();
            }
        });

    }
}
