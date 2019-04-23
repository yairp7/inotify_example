package com.pech.tools.inotify_example;

import android.support.v7.app.AppCompatActivity;
import android.os.Bundle;
import android.widget.TextView;

public class MainActivity extends AppCompatActivity {

    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        setContentView(R.layout.activity_main);

        TextView tv = findViewById(R.id.sample_text);

        initLib();
        addWatch("/", true);
        start();
    }

    // Used to load the 'native-lib' library on application startup.
    static {
        System.loadLibrary("native-lib");
    }

    @Override
    protected void onDestroy() {
        super.onDestroy();
        close();
    }

    public native boolean initLib();
    public native void start();
    public native boolean addWatch(String path, boolean isWatchSubDirs);
    public native void close();
}
