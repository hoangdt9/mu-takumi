package com.muonline.client;

import org.libsdl.app.SDLActivity;
import org.libsdl.app.MuLog;
import android.os.Bundle;
import android.os.Build;
import android.content.res.AssetManager;
import android.graphics.Color;
import android.view.View;
import android.view.WindowManager;
import java.io.File;
import java.io.FileOutputStream;
import java.io.InputStream;
import java.io.OutputStream;
import java.io.IOException;

/**
 * MuMain Android entry Activity.
 *
 * Extends SDL2's SDLActivity which:
 *   1. Loads native libraries (libSDL2.so, libmain.so, ...)
 *   2. Creates an OpenGL ES surface
 *   3. Forwards touch/key events to SDL2's JNI layer
 *   4. SDL2 then calls our SDL_main() in android_main.cpp
 */
public class MuMainActivity extends SDLActivity {

    private static final String TAG = "MuMain";

    // ─────────────────────────────────────────────────────────────────────────
    // Copy a single asset file from the APK to destFile on the real filesystem.
    // Skips if the file already exists (avoids unnecessary I/O on every launch).
    // ─────────────────────────────────────────────────────────────────────────
    private void copyAssetFile(AssetManager assetMgr, String srcAssetPath, File destFile) {
        if (destFile.exists()) return;   // already copied
        File parent = destFile.getParentFile();
        if (parent != null && !parent.exists()) {
            parent.mkdirs();
        }
        try (InputStream in = assetMgr.open(srcAssetPath);
             OutputStream out = new FileOutputStream(destFile)) {
            byte[] buf = new byte[8192];
            int len;
            while ((len = in.read(buf)) > 0) {
                out.write(buf, 0, len);
            }
            MuLog.i(TAG, "copyAsset OK: " + srcAssetPath + " -> " + destFile.getAbsolutePath());
        } catch (IOException e) {
            MuLog.i(TAG, "copyAsset FAIL: " + srcAssetPath + " : " + e.getMessage());
        }
    }

    // ─────────────────────────────────────────────────────────────────────────
    // Recursively copy all files inside an APK assets folder to destDir.
    // ─────────────────────────────────────────────────────────────────────────
    private void copyAssetFolder(AssetManager assetMgr, String srcFolder, File destDir) {
        String[] entries;
        try {
            entries = assetMgr.list(srcFolder);
        } catch (IOException e) {
            MuLog.i(TAG, "copyAssetFolder list FAIL: " + srcFolder + " : " + e.getMessage());
            return;
        }
        if (entries == null || entries.length == 0) return;
        if (!destDir.exists()) destDir.mkdirs();
        for (String name : entries) {
            String childSrc = srcFolder + "/" + name;
            File   childDst = new File(destDir, name);
            // Distinguish file vs sub-folder: if list() returns entries it's a folder
            String[] sub;
            try { sub = assetMgr.list(childSrc); } catch (IOException ex) { sub = null; }
            if (sub != null && sub.length > 0) {
                copyAssetFolder(assetMgr, childSrc, childDst);
            } else {
                copyAssetFile(assetMgr, childSrc, childDst);
            }
        }
    }

    // ─────────────────────────────────────────────────────────────────────────
    // Extract APK assets that the C++ layer needs to load as regular files.
    // Destination: getExternalFilesDir(null)  →
    //   /sdcard/Android/data/com.muonline.client/files/
    // That matches the chdir() in android_set_data_dir_early() so relative
    // paths like "ui/map.png" resolve correctly from C++.
    // ─────────────────────────────────────────────────────────────────────────
    private void extractGameAssets() {
        File extDir = getExternalFilesDir(null);
        if (extDir == null) {
            extDir = getFilesDir();   // internal storage fallback
        }
        MuLog.i(TAG, "extractGameAssets -> " + extDir.getAbsolutePath());
        AssetManager assetMgr = getAssets();
        // Copy assets/ui/ -> <extDir>/ui/
        copyAssetFolder(assetMgr, "ui", new File(extDir, "ui"));
    }

    @Override
    protected void onCreate(Bundle savedInstanceState) {
        MuLog.i(TAG, "MuMainActivity onCreate");

        // Extract bundled assets to the filesystem so C++ can open them with
        // normal fopen / stbi_load. Must run BEFORE super.onCreate() loads libs.
        extractGameAssets();

        // Must be set BEFORE super.onCreate() so SDL creates EGL surface at
        // the correct full-screen size (avoids SurfaceFlinger buffer rejection).
        configureFullscreenWindow();

        super.onCreate(savedInstanceState);
    }

    private void configureFullscreenWindow() {
        getWindow().addFlags(WindowManager.LayoutParams.FLAG_KEEP_SCREEN_ON
            | WindowManager.LayoutParams.FLAG_FULLSCREEN);

        if (Build.VERSION.SDK_INT >= Build.VERSION_CODES.P) {
            WindowManager.LayoutParams attrs = getWindow().getAttributes();
            attrs.layoutInDisplayCutoutMode =
                WindowManager.LayoutParams.LAYOUT_IN_DISPLAY_CUTOUT_MODE_SHORT_EDGES;
            getWindow().setAttributes(attrs);
        }

        if (Build.VERSION.SDK_INT >= Build.VERSION_CODES.R) {
            getWindow().setDecorFitsSystemWindows(false);
        }

        getWindow().setStatusBarColor(Color.TRANSPARENT);
        getWindow().setNavigationBarColor(Color.TRANSPARENT);
        applyImmersiveFlags();
    }

    private void applyImmersiveFlags() {
        getWindow().getDecorView().setSystemUiVisibility(
            View.SYSTEM_UI_FLAG_IMMERSIVE_STICKY
                | View.SYSTEM_UI_FLAG_LAYOUT_STABLE
                | View.SYSTEM_UI_FLAG_LAYOUT_HIDE_NAVIGATION
                | View.SYSTEM_UI_FLAG_LAYOUT_FULLSCREEN
                | View.SYSTEM_UI_FLAG_HIDE_NAVIGATION
                | View.SYSTEM_UI_FLAG_FULLSCREEN);
    }

    /**
     * Native libraries to load in order.
     * These .so files must exist in jniLibs/<ABI>/ for each packaged ABI.
     * Import prebuilt binaries with android/copy_libs.bat.
     */
    @Override
    protected String[] getLibraries() {
        return new String[]{
            "mpg123",       // MP3 decoder (no prefix in System.loadLibrary)
            "SDL2",         // Window, input, events
            "SDL2_image",   // Texture loading (PNG, JPG)
            "SDL2_mixer",   // Audio mixing (replaces DirectSound + wzAudio)
            "SDL2_ttf",     // Font rendering
            "SDL_net",      // Network (replaces WinSock2)
            "botan",        // Cryptography (replaces Windows DPAPI)
            "main"          // Our game - libmain.so (compiled from MuMain sources)
        };
    }

    @Override
    public void onWindowFocusChanged(boolean hasFocus) {
        super.onWindowFocusChanged(hasFocus);
        if (hasFocus) {
            configureFullscreenWindow();
        }
    }

    @Override
    protected void onPause() {
        MuLog.d(TAG, "onPause");
        super.onPause();
    }

    @Override
    protected void onResume() {
        MuLog.d(TAG, "onResume");
        super.onResume();
    }

    @Override
    protected void onDestroy() {
        MuLog.i(TAG, "onDestroy");
        super.onDestroy();
    }
}
