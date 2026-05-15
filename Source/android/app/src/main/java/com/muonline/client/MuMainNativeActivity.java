package com.muonline.client;

import android.app.NativeActivity;
import android.content.Context;
import android.content.res.AssetManager;
import android.graphics.Color;
import android.graphics.PixelFormat;
import android.graphics.SurfaceTexture;
import android.media.MediaPlayer;
import android.util.Log;
import android.view.Surface;
import android.view.TextureView;
import android.os.Build;
import android.os.Bundle;
import android.text.InputType;
import android.view.KeyEvent;
import android.view.View;
import android.view.ViewGroup;
import android.view.WindowManager;
import android.view.inputmethod.EditorInfo;
import android.view.inputmethod.InputConnection;
import android.view.inputmethod.InputConnectionWrapper;
import android.view.inputmethod.InputMethodManager;
import android.widget.EditText;
import android.widget.FrameLayout;

import java.io.File;
import java.io.FileOutputStream;
import java.io.IOException;
import java.io.InputStream;
import java.io.OutputStream;

public class MuMainNativeActivity extends NativeActivity {
    static {
        System.loadLibrary("main");
    }

    /** Build-time / Gradle overrides for first TCP hop (see app/build.gradle). */
    private static native void nativeApplyNetworkBootstrap(String host, int port);

    private static native void nativeOnTextInput(String text);
    private static native void nativeOnKeyEvent(
        int action,
        int keyCode,
        int unicodeChar,
        int metaState,
        int repeatCount);

    /** IME action bar (Done / Go / Next): same as pressing Return on a physical keyboard. */
    private static native void nativeOnImeEditorAction(int actionCode);

    /** Called when Android intro video (MediaPlayer) ends or is dismissed — unblocks native login scene. */
    private native void nativeLoginMovieComplete();

    private static native void nativeLoginBackgroundMovieStopped();

    private BridgeEditText imeBridge;

    private static final String TAG_MOVIE = "TakumiLoginMovie";
    private FrameLayout loginMovieOverlay;
    private TextureView loginMovieTexture;
    private MediaPlayer loginMoviePlayer;

    /** Login loop video: MediaPlayer -> SurfaceTexture bound to native GL_TEXTURE_EXTERNAL_OES (see MobilePlatform). */
    private SurfaceTexture loginBgGlSurfaceTexture;
    private MediaPlayer loginBgGlMediaPlayer;

    private void resetImeBridgeBuffer() {
        if (imeBridge == null) {
            return;
        }
        imeBridge.setText("");
        imeBridge.setSelection(imeBridge.getText().length());
    }

    private void installImeBridge() {
        if (imeBridge != null) {
            return;
        }

        imeBridge = new BridgeEditText(this);
        imeBridge.setBackgroundColor(Color.TRANSPARENT);
        imeBridge.setTextColor(Color.TRANSPARENT);
        imeBridge.setHighlightColor(Color.TRANSPARENT);
        imeBridge.setCursorVisible(false);
        imeBridge.setLongClickable(false);
        imeBridge.setTextIsSelectable(false);
        imeBridge.setFocusable(true);
        imeBridge.setFocusableInTouchMode(true);
        // Do not pop the soft keyboard on splash / gameplay; chat can call showSoftInput when needed.
        imeBridge.setShowSoftInputOnFocus(false);
        imeBridge.setSingleLine(true);
        imeBridge.setInputType(
            InputType.TYPE_CLASS_TEXT
                | InputType.TYPE_TEXT_FLAG_NO_SUGGESTIONS
                | InputType.TYPE_TEXT_VARIATION_VISIBLE_PASSWORD);
        imeBridge.setImeOptions(
            EditorInfo.IME_FLAG_NO_EXTRACT_UI
                | EditorInfo.IME_FLAG_NO_FULLSCREEN
                | EditorInfo.IME_ACTION_DONE);

        FrameLayout.LayoutParams params = new FrameLayout.LayoutParams(1, 1);
        params.leftMargin = 0;
        params.topMargin = 0;
        addContentView(imeBridge, params);
        imeBridge.post(
            () -> {
                View decor = getWindow() != null ? getWindow().getDecorView() : null;
                if (decor == null) {
                    return;
                }
                InputMethodManager imm =
                    (InputMethodManager) getSystemService(Context.INPUT_METHOD_SERVICE);
                if (imm != null) {
                    imm.hideSoftInputFromWindow(decor.getWindowToken(), 0);
                }
            });
    }

    /**
     * Invoked from native ({@code MobilePlatform}) when a Win32-style edit control gains focus
     * (login, chat). {@code ANativeActivity_showSoftInput} used by Sokol is unreliable here;
     * driving {@link InputMethodManager} on the invisible bridge view matches IME events to
     * {@link #nativeOnTextInput} / {@link #nativeOnKeyEvent}.
     */
    public void showImeBridgeKeyboard() {
        runOnUiThread(
            () -> {
                installImeBridge();
                if (imeBridge == null) {
                    return;
                }
                imeBridge.requestFocus();
                InputMethodManager imm =
                    (InputMethodManager) getSystemService(Context.INPUT_METHOD_SERVICE);
                if (imm != null) {
                    imm.showSoftInput(imeBridge, InputMethodManager.SHOW_IMPLICIT);
                }
            });
    }

    /** Invoked from native when text input mode ends. */
    public void hideImeBridgeKeyboard() {
        runOnUiThread(
            () -> {
                if (imeBridge == null) {
                    return;
                }
                InputMethodManager imm =
                    (InputMethodManager) getSystemService(Context.INPUT_METHOD_SERVICE);
                if (imm != null) {
                    imm.hideSoftInputFromWindow(imeBridge.getWindowToken(), 0);
                }
                imeBridge.clearFocus();
                View decor = getWindow() != null ? getWindow().getDecorView() : null;
                if (decor != null) {
                    decor.requestFocus();
                }
            });
    }

    /**
     * JNI from native: same intro asset as main PC client ({@code Data/Movie/MU.wmv}, see {@code MOVIE_FILE_WMV}).
     * Path is absolute on device storage after data unpack.
     */
    public void playLoginIntroMovie(String absolutePath) {
        runOnUiThread(
            () -> {
                stopLoginIntroMovieInternal(false);
                if (absolutePath == null || absolutePath.isEmpty()) {
                    Log.w(TAG_MOVIE, "playLoginIntroMovie: empty path");
                    return;
                }
                loginMovieOverlay = new FrameLayout(this);
                loginMovieOverlay.setLayoutParams(
                    new FrameLayout.LayoutParams(
                        ViewGroup.LayoutParams.MATCH_PARENT, ViewGroup.LayoutParams.MATCH_PARENT));
                loginMovieOverlay.setClickable(true);
                loginMovieOverlay.setOnClickListener(v -> stopLoginIntroMovieInternal(true));

                loginMovieTexture = new TextureView(this);
                loginMovieTexture.setLayoutParams(
                    new FrameLayout.LayoutParams(
                        ViewGroup.LayoutParams.MATCH_PARENT, ViewGroup.LayoutParams.MATCH_PARENT));
                loginMovieOverlay.addView(loginMovieTexture);
                loginMovieOverlay.setBackgroundColor(Color.BLACK);

                addContentView(
                    loginMovieOverlay,
                    new FrameLayout.LayoutParams(
                        ViewGroup.LayoutParams.MATCH_PARENT, ViewGroup.LayoutParams.MATCH_PARENT));

                loginMovieTexture.setSurfaceTextureListener(
                    new TextureView.SurfaceTextureListener() {
                        @Override
                        public void onSurfaceTextureAvailable(SurfaceTexture surfaceTexture, int w, int h) {
                            try {
                                loginMoviePlayer = new MediaPlayer();
                                loginMoviePlayer.setDataSource(absolutePath);
                                loginMoviePlayer.setSurface(new Surface(surfaceTexture));
                                loginMoviePlayer.setOnCompletionListener(
                                    mp -> runOnUiThread(() -> stopLoginIntroMovieInternal(true)));
                                loginMoviePlayer.setOnErrorListener(
                                    (mp, what, extra) -> {
                                        Log.e(TAG_MOVIE, "MediaPlayer error what=" + what + " extra=" + extra);
                                        runOnUiThread(() -> stopLoginIntroMovieInternal(true));
                                        return true;
                                    });
                                loginMoviePlayer.prepare();
                                loginMoviePlayer.start();
                            } catch (Exception e) {
                                Log.e(TAG_MOVIE, "playLoginIntroMovie failed: " + e.getMessage());
                                stopLoginIntroMovieInternal(true);
                            }
                        }

                        @Override
                        public void onSurfaceTextureSizeChanged(SurfaceTexture surfaceTexture, int w, int h) {}

                        @Override
                        public boolean onSurfaceTextureDestroyed(SurfaceTexture surfaceTexture) {
                            stopLoginIntroMovieInternal(false);
                            return true;
                        }

                        @Override
                        public void onSurfaceTextureUpdated(SurfaceTexture surfaceTexture) {}
                    });
            });
    }

    /**
     * Called from the render thread with the current EGL context. Binds {@code textureId} as
     * GL_TEXTURE_EXTERNAL_OES and attaches a looping muted {@link MediaPlayer}.
     */
    public boolean bindLoginBackgroundMovieToGlTexture(int textureId, String absolutePath) {
        releaseLoginBackgroundMovieGlInternal();
        if (absolutePath == null || absolutePath.isEmpty()) {
            Log.w(TAG_MOVIE, "bindLoginBackgroundMovieToGlTexture: empty path");
            return false;
        }
        try {
            loginBgGlSurfaceTexture = new SurfaceTexture(textureId);
            loginBgGlSurfaceTexture.setDefaultBufferSize(2, 2);
            loginBgGlMediaPlayer = new MediaPlayer();
            loginBgGlMediaPlayer.setSurface(new Surface(loginBgGlSurfaceTexture));
            loginBgGlMediaPlayer.setLooping(true);
            loginBgGlMediaPlayer.setVolume(0f, 0f);
            loginBgGlMediaPlayer.setOnPreparedListener(
                mp -> {
                    try {
                        int w = mp.getVideoWidth();
                        int h = mp.getVideoHeight();
                        if (w > 0 && h > 0 && loginBgGlSurfaceTexture != null) {
                            loginBgGlSurfaceTexture.setDefaultBufferSize(w, h);
                        }
                    } catch (Exception ignored) {
                    }
                });
            loginBgGlMediaPlayer.setOnErrorListener(
                (mp, what, extra) -> {
                    Log.e(TAG_MOVIE, "login bg GL MediaPlayer error what=" + what + " extra=" + extra);
                    nativeLoginBackgroundMovieStopped();
                    releaseLoginBackgroundMovieGlInternal();
                    return true;
                });
            loginBgGlMediaPlayer.setDataSource(absolutePath);
            loginBgGlMediaPlayer.prepare();
            loginBgGlMediaPlayer.start();
            return true;
        } catch (Exception e) {
            Log.e(TAG_MOVIE, "bindLoginBackgroundMovieToGlTexture failed: " + e.getMessage());
            releaseLoginBackgroundMovieGlInternal();
            return false;
        }
    }

    public void updateLoginBackgroundMovieTexture() {
        if (loginBgGlSurfaceTexture != null) {
            try {
                loginBgGlSurfaceTexture.updateTexImage();
            } catch (Exception e) {
                Log.e(TAG_MOVIE, "updateLoginBackgroundMovieTexture: " + e.getMessage());
            }
        }
    }

    private void releaseLoginBackgroundMovieGlInternal() {
        if (loginBgGlMediaPlayer != null) {
            try {
                loginBgGlMediaPlayer.stop();
            } catch (Exception ignored) {
            }
            try {
                loginBgGlMediaPlayer.release();
            } catch (Exception ignored) {
            }
            loginBgGlMediaPlayer = null;
        }
        if (loginBgGlSurfaceTexture != null) {
            try {
                loginBgGlSurfaceTexture.release();
            } catch (Exception ignored) {
            }
            loginBgGlSurfaceTexture = null;
        }
    }

    /** Release MediaPlayer/SurfaceTexture only. Native performs glDeleteTextures (render thread or deferred). */
    public void releaseLoginBackgroundMovieGl() {
        releaseLoginBackgroundMovieGlInternal();
    }

    public void stopLoginBackgroundMovie() {
        releaseLoginBackgroundMovieGl();
    }

    public void stopLoginIntroMovie() {
        runOnUiThread(() -> stopLoginIntroMovieInternal(true));
    }

    private void stopLoginIntroMovieInternal(boolean notifyNativeComplete) {
        final boolean hadSomething = (loginMoviePlayer != null || loginMovieOverlay != null);
        if (loginMoviePlayer != null) {
            try {
                loginMoviePlayer.stop();
            } catch (Exception ignored) {
            }
            try {
                loginMoviePlayer.release();
            } catch (Exception ignored) {
            }
            loginMoviePlayer = null;
        }
        if (loginMovieOverlay != null) {
            ViewGroup parent = (ViewGroup) loginMovieOverlay.getParent();
            if (parent != null) {
                parent.removeView(loginMovieOverlay);
            }
        }
        loginMovieOverlay = null;
        loginMovieTexture = null;
        if (notifyNativeComplete && hadSomething) {
            nativeLoginMovieComplete();
        }
    }

    private final class BridgeEditText extends EditText {
        BridgeEditText(MuMainNativeActivity activity) {
            super(activity);
        }

        private void forwardKeyEvent(KeyEvent event) {
            if (event == null) {
                return;
            }
            nativeOnKeyEvent(
                event.getAction(),
                event.getKeyCode(),
                event.getUnicodeChar(),
                event.getMetaState(),
                event.getRepeatCount());
        }

        @Override
        public boolean onCheckIsTextEditor() {
            return true;
        }

        @Override
        public InputConnection onCreateInputConnection(EditorInfo outAttrs) {
            outAttrs.imeOptions |= EditorInfo.IME_FLAG_NO_EXTRACT_UI | EditorInfo.IME_FLAG_NO_FULLSCREEN;
            final InputConnection baseConnection = super.onCreateInputConnection(outAttrs);
            return new InputConnectionWrapper(baseConnection, true) {
                @Override
                public boolean commitText(CharSequence text, int newCursorPosition) {
                    if (text != null && text.length() > 0) {
                        nativeOnTextInput(text.toString());
                    }
                    post(new Runnable() {
                        @Override
                        public void run() {
                            resetImeBridgeBuffer();
                        }
                    });
                    return true;
                }

                @Override
                public boolean setComposingText(CharSequence text, int newCursorPosition) {
                    if (text != null && text.length() > 0) {
                        nativeOnTextInput(text.toString());
                    }
                    post(new Runnable() {
                        @Override
                        public void run() {
                            resetImeBridgeBuffer();
                        }
                    });
                    return true;
                }

                @Override
                public boolean deleteSurroundingText(int beforeLength, int afterLength) {
                    if (beforeLength > 0) {
                        nativeOnKeyEvent(KeyEvent.ACTION_DOWN, KeyEvent.KEYCODE_DEL, 0, 0, 0);
                        nativeOnKeyEvent(KeyEvent.ACTION_UP, KeyEvent.KEYCODE_DEL, 0, 0, 0);
                        return true;
                    }
                    return super.deleteSurroundingText(beforeLength, afterLength);
                }

                @Override
                public boolean sendKeyEvent(KeyEvent event) {
                    forwardKeyEvent(event);
                    return true;
                }

                @Override
                public boolean performEditorAction(int actionCode) {
                    // Do not inject "\\n" as text (pollutes password fields). Native maps to SDL Return / default OK.
                    nativeOnImeEditorAction(actionCode);
                    return true;
                }
            };
        }

        @Override
        public boolean onKeyDown(int keyCode, KeyEvent event) {
            forwardKeyEvent(event);
            return true;
        }

        @Override
        public boolean onKeyUp(int keyCode, KeyEvent event) {
            forwardKeyEvent(event);
            return true;
        }

        @Override
        public boolean onKeyPreIme(int keyCode, KeyEvent event) {
            if (keyCode == KeyEvent.KEYCODE_BACK) {
                forwardKeyEvent(event);
                return true;
            }
            return super.onKeyPreIme(keyCode, event);
        }
    }


    private void copyAssetFile(AssetManager assetMgr, String srcAssetPath, File destFile) {
        if (destFile.exists()) {
            return;
        }
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
        } catch (IOException ignored) {
        }
    }

    private void copyAssetFolder(AssetManager assetMgr, String srcFolder, File destDir) {
        String[] entries;
        try {
            entries = assetMgr.list(srcFolder);
        } catch (IOException e) {
            return;
        }
        if (entries == null || entries.length == 0) {
            return;
        }
        if (!destDir.exists()) {
            destDir.mkdirs();
        }
        for (String name : entries) {
            String childSrc = srcFolder + "/" + name;
            File childDst = new File(destDir, name);
            String[] sub;
            try {
                sub = assetMgr.list(childSrc);
            } catch (IOException ex) {
                sub = null;
            }
            if (sub != null && sub.length > 0) {
                copyAssetFolder(assetMgr, childSrc, childDst);
            } else {
                copyAssetFile(assetMgr, childSrc, childDst);
            }
        }
    }

    private void extractGameAssets() {
        File extDir = getExternalFilesDir(null);
        if (extDir == null) {
            extDir = getFilesDir();
        }
        copyAssetFolder(getAssets(), "ui", new File(extDir, "ui"));
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
        getWindow().setFormat(PixelFormat.TRANSLUCENT);
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

    @Override
    protected void onPause() {
        stopLoginIntroMovieInternal(true);
        nativeLoginBackgroundMovieStopped();
        releaseLoginBackgroundMovieGl();
        super.onPause();
    }

    @Override
    protected void onCreate(Bundle savedInstanceState) {
        nativeApplyNetworkBootstrap(
            BuildConfig.MU_BOOTSTRAP_SERVER_HOST,
            BuildConfig.MU_BOOTSTRAP_SERVER_PORT);
        extractGameAssets();
        configureFullscreenWindow();
        super.onCreate(savedInstanceState);
        installImeBridge();
    }

    @Override
    public void onWindowFocusChanged(boolean hasFocus) {
        super.onWindowFocusChanged(hasFocus);
        if (hasFocus) {
            configureFullscreenWindow();
        }
    }
}
