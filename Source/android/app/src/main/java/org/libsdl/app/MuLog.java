package org.libsdl.app;

import android.util.Log;

import com.muonline.client.BuildConfig;

public final class MuLog {
    private static final boolean ENABLED = BuildConfig.ENABLE_LOGS;

    private MuLog() {
    }

    public static int v(String tag, String msg) {
        return ENABLED ? Log.v(tag, msg) : 0;
    }

    public static int v(String tag, String msg, Throwable tr) {
        return ENABLED ? Log.v(tag, msg, tr) : 0;
    }

    public static int d(String tag, String msg) {
        return ENABLED ? Log.d(tag, msg) : 0;
    }

    public static int d(String tag, String msg, Throwable tr) {
        return ENABLED ? Log.d(tag, msg, tr) : 0;
    }

    public static int i(String tag, String msg) {
        return ENABLED ? Log.i(tag, msg) : 0;
    }

    public static int i(String tag, String msg, Throwable tr) {
        return ENABLED ? Log.i(tag, msg, tr) : 0;
    }

    public static int w(String tag, String msg) {
        return ENABLED ? Log.w(tag, msg) : 0;
    }

    public static int w(String tag, String msg, Throwable tr) {
        return ENABLED ? Log.w(tag, msg, tr) : 0;
    }

    public static int e(String tag, String msg) {
        return ENABLED ? Log.e(tag, msg) : 0;
    }

    public static int e(String tag, String msg, Throwable tr) {
        return ENABLED ? Log.e(tag, msg, tr) : 0;
    }

    public static String getStackTraceString(Throwable tr) {
        return Log.getStackTraceString(tr);
    }
}
