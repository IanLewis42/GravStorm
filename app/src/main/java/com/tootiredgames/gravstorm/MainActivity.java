package com.tootiredgames.gravstorm;

import android.app.Activity;
import android.content.pm.ActivityInfo;
import android.os.Build;
import android.os.Bundle;
import android.os.VibrationEffect;
import android.widget.TextView;
import android.view.View;

import org.liballeg.android.AllegroActivity;

import android.view.inputmethod.InputMethodManager;
import android.content.Context;
import android.view.KeyEvent;

import android.os.Handler;
import android.os.Looper;
import android.os.Vibrator;

public class MainActivity extends AllegroActivity {

    static {
        System.loadLibrary("allegro");
        System.loadLibrary("allegro_primitives");
        System.loadLibrary("allegro_image");
        System.loadLibrary("allegro_font");
        System.loadLibrary("allegro_ttf");
        System.loadLibrary("allegro_audio");
        System.loadLibrary("allegro_acodec");
        System.loadLibrary("allegro_color");
    }

    // Vibrate
    private void shakeItBaby(int time) {
        if (Build.VERSION.SDK_INT >= 26) {
            ((Vibrator) getSystemService(VIBRATOR_SERVICE)).vibrate(VibrationEffect.createOneShot(time, VibrationEffect.DEFAULT_AMPLITUDE));
        } else {
            ((Vibrator) getSystemService(VIBRATOR_SERVICE)).vibrate(time);
        }
    }

    public void OpenKeyBoard() {
        Context mContext = getApplicationContext();
        InputMethodManager imm = (InputMethodManager) mContext.getSystemService(mContext.INPUT_METHOD_SERVICE);
        imm.toggleSoftInput(InputMethodManager.SHOW_FORCED, 0);
    }

    public void CloseKeyBoard() {
        Context mContext = getApplicationContext();
        InputMethodManager imm = (InputMethodManager) mContext.getSystemService(mContext.INPUT_METHOD_SERVICE);
        imm.toggleSoftInput(InputMethodManager.HIDE_IMPLICIT_ONLY, 0);

        new Handler(Looper.getMainLooper()).postDelayed(new Runnable() {
            @Override
            public void run() {
                View decorView = getWindow().getDecorView();
                decorView.setSystemUiVisibility(
                        View.SYSTEM_UI_FLAG_LAYOUT_STABLE
                                | View.SYSTEM_UI_FLAG_LAYOUT_HIDE_NAVIGATION
                                | View.SYSTEM_UI_FLAG_LAYOUT_FULLSCREEN
                                | View.SYSTEM_UI_FLAG_HIDE_NAVIGATION
                                | View.SYSTEM_UI_FLAG_FULLSCREEN
                                | View.SYSTEM_UI_FLAG_IMMERSIVE_STICKY);

            }}, 100);
    }



    @Override
    public void onWindowFocusChanged(boolean hasFocus) {
        View decorView = this.getWindow().getDecorView();
        super.onWindowFocusChanged(hasFocus);
        if (hasFocus) {
            decorView.setSystemUiVisibility(
                    View.SYSTEM_UI_FLAG_LAYOUT_STABLE
                            | View.SYSTEM_UI_FLAG_LAYOUT_HIDE_NAVIGATION
                            | View.SYSTEM_UI_FLAG_LAYOUT_FULLSCREEN
                            | View.SYSTEM_UI_FLAG_HIDE_NAVIGATION
                            | View.SYSTEM_UI_FLAG_FULLSCREEN
                            | View.SYSTEM_UI_FLAG_IMMERSIVE_STICKY);
        }
    }

    public static boolean myvar = false;

    // Declare a native method sayHello() that receives nothing and returns void
    //private native void SystemBackPressed();

    // Test Driver
    //public static void main(String[] args) {
    //    new MainActivity().sayHello();  // invoke the native method

    @Override
    public void  onBackPressed() {
        //if (mWebView.canGoBack()) {
        //    mWebView.goBack();

            myvar = true;

        //return true;

        //new MainActivity().SystemBackPressed();
            //return;
        //}

        // Otherwise defer to system default behavior.
        //super.onBackPressed();


    }
/*
    public int WasBackPressed() {
        if (SystemBackPressed) {
            return 1;
            SystemBackPressed = false;
        }
        else
            return 0;
    }
*/

    @Override
    public boolean onKeyDown(int keyCode, KeyEvent event)
    {
        //replaces the default 'Back' button action
        if(keyCode==KeyEvent.KEYCODE_BACK)   {
// something here

            finish();
        }
        return true;
    }

    public MainActivity()
    {
        super("libnative-lib.so");
    }
}




