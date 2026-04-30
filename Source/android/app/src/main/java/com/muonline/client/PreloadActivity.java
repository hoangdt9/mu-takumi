package com.muonline.client;

import android.app.Activity;
import android.content.Intent;
import android.graphics.Color;
import android.graphics.drawable.ClipDrawable;
import android.graphics.drawable.Drawable;
import android.graphics.drawable.GradientDrawable;
import android.graphics.drawable.LayerDrawable;
import android.os.Bundle;
import android.os.Build;
import android.os.Environment;
import android.os.Handler;
import android.os.Looper;
import android.os.SystemClock;
import android.util.Base64;
import android.util.Log;
import android.view.Gravity;
import android.view.View;
import android.view.WindowManager;
import android.widget.FrameLayout;
import android.widget.ImageView;
import android.widget.LinearLayout;
import android.widget.ProgressBar;
import android.widget.TextView;

import java.io.BufferedInputStream;
import java.io.BufferedOutputStream;
import java.io.File;
import java.io.FileInputStream;
import java.io.FileOutputStream;
import java.io.IOException;
import java.io.InputStream;
import java.io.OutputStream;
import java.net.HttpURLConnection;
import java.net.URL;
import java.nio.charset.StandardCharsets;
import java.util.ArrayList;
import java.util.Enumeration;
import java.util.List;
import java.util.Locale;
import java.util.concurrent.ExecutorService;
import java.util.concurrent.Executors;
import java.util.zip.ZipEntry;
import java.util.zip.ZipFile;

public class PreloadActivity extends Activity {

    private static final String TAG = "MuPreload";
    private static final String[] DATA_ZIP_URL_CANDIDATES = {
        "http://update.daybreak.id.vn/update/data.zip"
    };
    private static final String BASIC_AUTH_USERNAME = "admin";
    private static final String BASIC_AUTH_PASSWORD = "openmu";
    private static final String DATA_FOLDER_NAME = "Data";
    private static final String DATA_READY_MARKER_FILE = ".mu_data_ready_v1";
    private static final String[] DATA_DIR_HINTS = {
        "Local",
        "Item",
        "Skill",
        "World1",
        "Object1"
    };
    private static final String[] TAKUMI_REQUIRED_FILES = {
        "Local/Mix.bmd",
        "Local/Item.bmd",
        "Local/Filter.bmd",
        "Local/FilterName.bmd",
        "Local/MonsterSkill.bmd",
        "Local/MasterSkillTreeData.bmd",
        "Local/Eng/Dialog_Eng.bmd",
        "Local/Eng/Item_Eng.bmd",
        "Local/Eng/itemtooltip_Eng.bmd",
        "Local/Eng/itemleveltooltip_Eng.bmd",
        "Local/Eng/itemtooltiptext_Eng.bmd",
        "Local/Eng/movereq_Eng.bmd",
        "Local/Eng/NpcName(Eng).txt",
        "Local/Eng/Quest_Eng.bmd",
        "Local/Eng/Skill_Eng.bmd",
        "Local/Eng/SocketItem_Eng.bmd",
        "Local/Eng/MasterSkillTooltip_Eng.bmd",
        "Local/Eng/Text_Eng.bmd",
        "Item/bsummoners01.ozj",
        "Item/bsummoners02_render.OZJ",
        "Item/bsummoners02.ozj",
        "Item/sd_soulsummoner.bmd",
        "Item/sd_soulsummonerstickA01.ozj",
        "Item/sd_soulsummonerstickA02_render.ozj",
        "Item/sd_soulsummonerstickA02.ozj",
        "Gate.bmd",
        "Macro.txt"
    };
    private static final String[] TAKUMI_REQUIRED_DIRS = {
        "Custom",
        "Effect",
        "Interface",
        "Item",
        "Local/Eng",
        "Monster",
        "Object1",
        "Player",
        "Skill",
        "World1"
    };
    private static final int[] BANNER_RES_IDS = {
        R.mipmap.banner_preload_1,
        R.mipmap.banner_preload_2
    };
    private static final long BANNER_SWITCH_MS = 2400L;
    private static final long UI_UPDATE_INTERVAL_MS = 110L;
    private static final int BUFFER_SIZE = 128 * 1024;

    private static final double DOWNLOAD_WEIGHT = 72.0;
    private static final double PARSE_WEIGHT = 28.0;

    private final Handler mainHandler = new Handler(Looper.getMainLooper());
    private final ExecutorService ioExecutor = Executors.newSingleThreadExecutor();

    private ImageView bannerView;
    private ProgressBar progressBar;
    private TextView stageText;
    private TextView detailText;
    private TextView timerText;
    private int currentBannerIndex = 0;
    private volatile boolean cancelled = false;

    private final Runnable bannerSwitcher = new Runnable() {
        @Override
        public void run() {
            if (cancelled || bannerView == null) {
                return;
            }
            currentBannerIndex = (currentBannerIndex + 1) % BANNER_RES_IDS.length;
            bannerView.animate()
                .alpha(0.08f)
                .setDuration(220L)
                .withEndAction(() -> {
                    if (cancelled || bannerView == null) {
                        return;
                    }
                    bannerView.setImageResource(BANNER_RES_IDS[currentBannerIndex]);
                    bannerView.animate().alpha(1.0f).setDuration(260L).start();
                })
                .start();
            mainHandler.postDelayed(this, BANNER_SWITCH_MS);
        }
    };

    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        configureWindow();
        buildUi();
        stageText.setText("Preparing preload...");
        detailText.setText(DATA_ZIP_URL_CANDIDATES[0]);
        timerText.setText("Waiting for network...");
        mainHandler.postDelayed(bannerSwitcher, BANNER_SWITCH_MS);
        ioExecutor.execute(this::runPreloadFlow);
    }

    @Override
    protected void onDestroy() {
        cancelled = true;
        mainHandler.removeCallbacksAndMessages(null);
        ioExecutor.shutdownNow();
        super.onDestroy();
    }

    private void configureWindow() {
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
        getWindow().getDecorView().setSystemUiVisibility(
            View.SYSTEM_UI_FLAG_IMMERSIVE_STICKY
                | View.SYSTEM_UI_FLAG_LAYOUT_STABLE
                | View.SYSTEM_UI_FLAG_LAYOUT_HIDE_NAVIGATION
                | View.SYSTEM_UI_FLAG_LAYOUT_FULLSCREEN
                | View.SYSTEM_UI_FLAG_HIDE_NAVIGATION
                | View.SYSTEM_UI_FLAG_FULLSCREEN);
    }

    private void buildUi() {
        FrameLayout root = new FrameLayout(this);
        root.setBackgroundColor(Color.parseColor("#080B14"));

        bannerView = new ImageView(this);
        bannerView.setScaleType(ImageView.ScaleType.CENTER_CROP);
        bannerView.setImageResource(BANNER_RES_IDS[currentBannerIndex]);
        bannerView.setAlpha(1.0f);
        root.addView(bannerView, new FrameLayout.LayoutParams(
            FrameLayout.LayoutParams.MATCH_PARENT,
            FrameLayout.LayoutParams.MATCH_PARENT
        ));

        View topShade = new View(this);
        topShade.setBackground(new GradientDrawable(
            GradientDrawable.Orientation.TOP_BOTTOM,
            new int[]{0x22000000, 0xB8000000}
        ));
        root.addView(topShade, new FrameLayout.LayoutParams(
            FrameLayout.LayoutParams.MATCH_PARENT,
            FrameLayout.LayoutParams.MATCH_PARENT
        ));

        LinearLayout panel = new LinearLayout(this);
        panel.setOrientation(LinearLayout.VERTICAL);
        int padH = dp(18);
        int padV = dp(14);
        panel.setPadding(padH, padV, padH, padV);
        panel.setBackground(createPanelBackground());

        TextView title = new TextView(this);
        title.setText("MU DATA SYNC");
        title.setTextSize(18f);
        title.setTextColor(Color.parseColor("#F3F8FF"));
        title.setLetterSpacing(0.06f);
        title.setTypeface(title.getTypeface(), android.graphics.Typeface.BOLD);
        panel.addView(title);

        stageText = new TextView(this);
        stageText.setTextSize(14f);
        stageText.setTextColor(Color.parseColor("#DCE8FF"));
        LinearLayout.LayoutParams stageLp = new LinearLayout.LayoutParams(
            LinearLayout.LayoutParams.MATCH_PARENT,
            LinearLayout.LayoutParams.WRAP_CONTENT
        );
        stageLp.topMargin = dp(8);
        panel.addView(stageText, stageLp);

        progressBar = new ProgressBar(this, null, android.R.attr.progressBarStyleHorizontal);
        progressBar.setMax(1000);
        progressBar.setProgress(0);
        progressBar.setIndeterminate(false);
        progressBar.setProgressDrawable(createProgressDrawable());
        LinearLayout.LayoutParams progressLp = new LinearLayout.LayoutParams(
            LinearLayout.LayoutParams.MATCH_PARENT,
            dp(16)
        );
        progressLp.topMargin = dp(10);
        panel.addView(progressBar, progressLp);

        detailText = new TextView(this);
        detailText.setTextSize(12f);
        detailText.setTextColor(Color.parseColor("#C8D6EA"));
        LinearLayout.LayoutParams detailLp = new LinearLayout.LayoutParams(
            LinearLayout.LayoutParams.MATCH_PARENT,
            LinearLayout.LayoutParams.WRAP_CONTENT
        );
        detailLp.topMargin = dp(8);
        panel.addView(detailText, detailLp);

        timerText = new TextView(this);
        timerText.setTextSize(12f);
        timerText.setTextColor(Color.parseColor("#B7C9E8"));
        LinearLayout.LayoutParams timerLp = new LinearLayout.LayoutParams(
            LinearLayout.LayoutParams.MATCH_PARENT,
            LinearLayout.LayoutParams.WRAP_CONTENT
        );
        timerLp.topMargin = dp(3);
        panel.addView(timerText, timerLp);

        FrameLayout.LayoutParams panelLp = new FrameLayout.LayoutParams(
            FrameLayout.LayoutParams.MATCH_PARENT,
            FrameLayout.LayoutParams.WRAP_CONTENT
        );
        panelLp.gravity = Gravity.BOTTOM;
        int margin = dp(20);
        panelLp.setMargins(margin, margin, margin, margin);
        root.addView(panel, panelLp);

        setContentView(root);
    }

    private Drawable createPanelBackground() {
        GradientDrawable panelBg = new GradientDrawable();
        panelBg.setShape(GradientDrawable.RECTANGLE);
        panelBg.setCornerRadius(dp(18));
        panelBg.setColor(Color.parseColor("#B21A2230"));
        panelBg.setStroke(dp(1), Color.parseColor("#52FFFFFF"));
        return panelBg;
    }

    private Drawable createProgressDrawable() {
        GradientDrawable track = new GradientDrawable();
        track.setShape(GradientDrawable.RECTANGLE);
        track.setCornerRadius(dp(999));
        track.setColor(Color.parseColor("#2DFFFFFF"));

        GradientDrawable fill = new GradientDrawable(
            GradientDrawable.Orientation.LEFT_RIGHT,
            new int[]{
                Color.parseColor("#FF5EC9FF"),
                Color.parseColor("#FF56FFA4"),
                Color.parseColor("#FFFFD85B")
            });
        fill.setCornerRadius(dp(999));

        ClipDrawable clipped = new ClipDrawable(fill, Gravity.START, ClipDrawable.HORIZONTAL);
        LayerDrawable layers = new LayerDrawable(new Drawable[]{track, clipped});
        layers.setId(0, android.R.id.background);
        layers.setId(1, android.R.id.progress);
        return layers;
    }

    private int dp(int value) {
        return Math.round(value * getResources().getDisplayMetrics().density);
    }

    private void runPreloadFlow() {
        final long totalStartMs = SystemClock.elapsedRealtime();
        List<File> dataRoots = collectDataRoots();
        File installRoot = null;
        File fallbackUsableRoot = null;
        try {
            checkCancelled();

            for (File root : dataRoots) {
                if (root == null) {
                    continue;
                }
                try {
                    ensureDirectory(root);
                } catch (IOException rootError) {
                    Log.w(TAG, "Skip data root " + root.getAbsolutePath() + ": " + rootError.getMessage());
                    continue;
                }

                if (installRoot == null) {
                    installRoot = root;
                }

                File existingDataDir = findChildDirectoryIgnoreCase(root, DATA_FOLDER_NAME);
                Log.i(TAG, "Check data root: " + root.getAbsolutePath()
                    + " existingDataDir=" + (existingDataDir != null ? existingDataDir.getAbsolutePath() : "null")
                    + " usable=" + isDataFolderUsable(existingDataDir)
                    + " marker=" + new File(root, DATA_READY_MARKER_FILE).exists());
                if (shouldSkipDownload(existingDataDir, root)) {
                    String dataPath = existingDataDir != null ? existingDataDir.getAbsolutePath() : "(unknown)";
                    postUi(() -> {
                        if (cancelled) {
                            return;
                        }
                        stageText.setText("Data already present. Skip download.");
                        detailText.setText(dataPath);
                        timerText.setText("Fast start mode");
                        progressBar.setProgress(1000);
                    });
                    mainHandler.postDelayed(this::launchGame, 350L);
                    return;
                }

                if (isDataFolderUsable(existingDataDir) && fallbackUsableRoot == null) {
                    fallbackUsableRoot = root;
                }
            }

            if (installRoot == null) {
                throw new IOException("No writable data root available.");
            }

            File preloadCacheRoot = new File(installRoot, "preload_data_sync");
            recreateDirectory(preloadCacheRoot);

            File zipFile = new File(preloadCacheRoot, "data.zip");
            DownloadStats downloadStats = downloadZip(zipFile);

            long parseStartMs = SystemClock.elapsedRealtime();
            File extractedRoot = new File(preloadCacheRoot, "extract_stage");
            recreateDirectory(extractedRoot);

            ZipMetrics zipMetrics = inspectZip(zipFile);
            extractZipCaseInsensitive(zipFile, extractedRoot, zipMetrics);
            installExtractedData(extractedRoot, installRoot);
            long parseDurationMs = SystemClock.elapsedRealtime() - parseStartMs;

            deleteRecursively(preloadCacheRoot);

            long totalDurationMs = SystemClock.elapsedRealtime() - totalStartMs;
            long finalParseDurationMs = parseDurationMs;
            postUi(() -> {
                if (cancelled) {
                    return;
                }
                progressBar.setProgress(1000);
                stageText.setText("Data ready. Launching game...");
                detailText.setText(
                    "Download: " + formatDuration(downloadStats.durationMs)
                        + " | Parse+Install: " + formatDuration(finalParseDurationMs));
                timerText.setText("Total sync time: " + formatDuration(totalDurationMs));
            });

            mainHandler.postDelayed(this::launchGame, 650L);
        } catch (Exception ex) {
            if (fallbackUsableRoot == null) {
                for (File root : dataRoots) {
                    File fallbackDataDir = findChildDirectoryIgnoreCase(root, DATA_FOLDER_NAME);
                    if (isDataFolderUsable(fallbackDataDir)) {
                        fallbackUsableRoot = root;
                        break;
                    }
                }
            }

            if (fallbackUsableRoot != null) {
                postUi(() -> {
                    if (cancelled) {
                        return;
                    }
                    stageText.setText("Sync failed, using existing data...");
                    detailText.setText(ex.getMessage() != null ? ex.getMessage() : "Unknown error");
                    timerText.setText("Launching with local data");
                    progressBar.setProgress(1000);
                });
                mainHandler.postDelayed(this::launchGame, 450L);
                return;
            }

            Log.e(TAG, "Preload failed", ex);
            postUi(() -> {
                if (cancelled) {
                    return;
                }
                stageText.setText("Preload failed");
                detailText.setText(ex.getMessage() != null ? ex.getMessage() : "Unknown error");
                timerText.setText("Please reopen app after checking server/port.");
            });
        }
    }

    private List<File> collectDataRoots() {
        List<File> roots = new ArrayList<>();

        File[] externalDirs = getExternalFilesDirs(null);
        if (externalDirs != null) {
            for (File dir : externalDirs) {
                addUniqueDataRoot(roots, dir);
            }
        }

        addUniqueDataRoot(roots, getExternalFilesDir(null));

        // Extra fallback for emulators/devices where getExternalFilesDir may return null intermittently.
        File legacyExternalRoot = new File(
            Environment.getExternalStorageDirectory(),
            "Android/data/" + getPackageName() + "/files");
        addUniqueDataRoot(roots, legacyExternalRoot);

        addUniqueDataRoot(roots, getFilesDir());
        return roots;
    }

    private void addUniqueDataRoot(List<File> roots, File candidate) {
        if (candidate == null) {
            return;
        }

        String candidatePath = safeCanonicalPath(candidate);
        for (File existing : roots) {
            String existingPath = safeCanonicalPath(existing);
            if (candidatePath.equals(existingPath)) {
                return;
            }
        }

        roots.add(candidate);
    }

    private static String safeCanonicalPath(File file) {
        if (file == null) {
            return "";
        }
        try {
            return file.getCanonicalPath();
        } catch (IOException ex) {
            return file.getAbsolutePath();
        }
    }

    private DownloadStats downloadZip(File outputZip) throws IOException {
        IOException lastError = null;
        for (String urlCandidate : DATA_ZIP_URL_CANDIDATES) {
            try {
                return downloadZipFromUrl(urlCandidate, false, outputZip);
            } catch (HttpStatusException httpEx) {
                if (httpEx.statusCode == 401) {
                    try {
                        return downloadZipFromUrl(urlCandidate, true, outputZip);
                    } catch (IOException authEx) {
                        lastError = authEx;
                    }
                } else {
                    lastError = httpEx;
                }
            } catch (IOException ex) {
                lastError = ex;
            }
        }

        if (lastError != null) {
            throw lastError;
        }
        throw new IOException("Unable to download data.zip from all configured URLs.");
    }

    private DownloadStats downloadZipFromUrl(String urlText, boolean withAuth, File outputZip) throws IOException {
        long startMs = SystemClock.elapsedRealtime();
        long downloadedBytes = 0L;
        long totalBytes = -1L;
        long lastUiTick = 0L;

        HttpURLConnection connection = null;
        try {
            postUi(() -> {
                if (cancelled) {
                    return;
                }
                stageText.setText(withAuth
                    ? "Connecting with Basic Auth..."
                    : "Connecting to download host...");
                detailText.setText(urlText);
                timerText.setText(withAuth
                    ? "Auth mode: admin/openmu"
                    : "Auth mode: none");
            });

            URL url = new URL(urlText);
            connection = (HttpURLConnection) url.openConnection();
            connection.setConnectTimeout(15000);
            connection.setReadTimeout(25000);
            connection.setRequestProperty("Accept-Encoding", "identity");
            connection.setRequestProperty("User-Agent", "MuMain-Android-Preload/1.0");
            connection.setInstanceFollowRedirects(true);
            if (withAuth) {
                applyBasicAuthorization(connection, BASIC_AUTH_USERNAME, BASIC_AUTH_PASSWORD);
            }
            connection.connect();

            int responseCode = connection.getResponseCode();
            if (responseCode < 200 || responseCode >= 300) {
                throw new HttpStatusException(
                    responseCode,
                    "HTTP " + responseCode + " while downloading data.zip (" + urlText + ")");
            }

            totalBytes = connection.getContentLengthLong();
            if (totalBytes == 0) {
                throw new IOException("Server returned empty data.zip");
            }

            try (InputStream rawStream = connection.getInputStream();
                 BufferedInputStream inputStream = new BufferedInputStream(rawStream, BUFFER_SIZE);
                 BufferedOutputStream outputStream = new BufferedOutputStream(new FileOutputStream(outputZip, false), BUFFER_SIZE)) {

                byte[] buffer = new byte[BUFFER_SIZE];
                int read;
                while ((read = inputStream.read(buffer)) != -1) {
                    checkCancelled();
                    outputStream.write(buffer, 0, read);
                    downloadedBytes += read;

                    long now = SystemClock.elapsedRealtime();
                    if (now - lastUiTick >= UI_UPDATE_INTERVAL_MS) {
                        final long elapsedMs = Math.max(1L, now - startMs);
                        double stageProgress = totalBytes > 0
                            ? Math.min(1.0, downloadedBytes / (double) totalBytes)
                            : 0.0;
                        double overallPercent = stageProgress * DOWNLOAD_WEIGHT;
                        updateDownloadUi(downloadedBytes, totalBytes, elapsedMs, overallPercent);
                        lastUiTick = now;
                    }
                }
                outputStream.flush();
            }
        } finally {
            if (connection != null) {
                connection.disconnect();
            }
        }

        long durationMs = SystemClock.elapsedRealtime() - startMs;
        updateDownloadUi(downloadedBytes, totalBytes, Math.max(1L, durationMs), DOWNLOAD_WEIGHT);
        return new DownloadStats(downloadedBytes, totalBytes, durationMs);
    }

    private static void applyBasicAuthorization(HttpURLConnection connection, String username, String password) {
        if (connection == null) {
            return;
        }
        String tokenRaw = username + ":" + password;
        String token = Base64.encodeToString(tokenRaw.getBytes(), Base64.NO_WRAP);
        connection.setRequestProperty("Authorization", "Basic " + token);
    }

    private ZipMetrics inspectZip(File zipFile) throws IOException {
        long totalBytes = 0L;
        int fileCount = 0;

        try (ZipFile zip = new ZipFile(zipFile)) {
            Enumeration<? extends ZipEntry> entries = zip.entries();
            while (entries.hasMoreElements()) {
                ZipEntry entry = entries.nextElement();
                if (entry.isDirectory()) {
                    continue;
                }
                fileCount++;
                long size = entry.getSize();
                if (size > 0) {
                    totalBytes += size;
                }
            }
        }

        return new ZipMetrics(totalBytes, fileCount);
    }

    private void extractZipCaseInsensitive(File zipFile, File extractRoot, ZipMetrics metrics) throws IOException {
        final long parseStartMs = SystemClock.elapsedRealtime();
        long extractedBytes = 0L;
        int extractedFiles = 0;
        long lastUiTick = 0L;

        try (ZipFile zip = new ZipFile(zipFile)) {
            Enumeration<? extends ZipEntry> entries = zip.entries();
            byte[] buffer = new byte[BUFFER_SIZE];

            while (entries.hasMoreElements()) {
                checkCancelled();
                ZipEntry entry = entries.nextElement();
                String normalizedName = normalizeZipPath(entry.getName());
                if (normalizedName.isEmpty()) {
                    continue;
                }

                boolean isDirectory = entry.isDirectory() || normalizedName.endsWith("/");
                File outFile = resolvePathCaseInsensitive(extractRoot, normalizedName, isDirectory);
                assertUnderRoot(extractRoot, outFile);

                if (isDirectory) {
                    ensureDirectory(outFile);
                    continue;
                }

                File parent = outFile.getParentFile();
                if (parent != null) {
                    ensureDirectory(parent);
                }

                try (InputStream inputStream = new BufferedInputStream(zip.getInputStream(entry), BUFFER_SIZE);
                     OutputStream outputStream = new BufferedOutputStream(new FileOutputStream(outFile, false), BUFFER_SIZE)) {
                    int read;
                    while ((read = inputStream.read(buffer)) != -1) {
                        checkCancelled();
                        outputStream.write(buffer, 0, read);
                        extractedBytes += read;

                        long now = SystemClock.elapsedRealtime();
                        if (now - lastUiTick >= UI_UPDATE_INTERVAL_MS) {
                            final long elapsedMs = Math.max(1L, now - parseStartMs);
                            double stageProgress = computeParseProgress(extractedBytes, extractedFiles, metrics);
                            double overallPercent = DOWNLOAD_WEIGHT + (stageProgress * PARSE_WEIGHT * 0.95);
                            updateExtractUi(extractedBytes, extractedFiles, metrics, elapsedMs, overallPercent);
                            lastUiTick = now;
                        }
                    }
                    outputStream.flush();
                }

                extractedFiles++;
            }
        }

        final long parseElapsedMs = Math.max(1L, SystemClock.elapsedRealtime() - parseStartMs);
        updateExtractUi(extractedBytes, extractedFiles, metrics, parseElapsedMs, DOWNLOAD_WEIGHT + (PARSE_WEIGHT * 0.95));
        updateInstallUi("Installing parsed data into old folder...");
    }

    private void installExtractedData(File extractRoot, File externalRoot) throws IOException {
        checkCancelled();

        File extractedDataDir = findChildDirectoryIgnoreCase(extractRoot, DATA_FOLDER_NAME);
        if (extractedDataDir == null) {
            File[] directories = extractRoot.listFiles(File::isDirectory);
            if (directories != null && directories.length == 1) {
                extractedDataDir = directories[0];
            }
        }
        if (extractedDataDir == null || !extractedDataDir.exists()) {
            throw new IOException("data folder not found in downloaded data.zip");
        }

        File existingDataDir = findChildDirectoryIgnoreCase(externalRoot, DATA_FOLDER_NAME);
        String targetName = existingDataDir != null ? existingDataDir.getName() : DATA_FOLDER_NAME;
        File targetDataDir = new File(externalRoot, targetName);

        boolean targetCleared = false;
        if (existingDataDir != null && existingDataDir.exists()) {
            targetCleared = tryDeleteExistingDataDir(existingDataDir);
        } else if (targetDataDir.exists()) {
            targetCleared = tryDeleteExistingDataDir(targetDataDir);
        } else {
            targetCleared = true;
        }

        if (targetCleared && moveDirectory(extractedDataDir, targetDataDir)) {
            // Fast-path: replace whole folder in one move.
        } else {
            // Fallback: merge-copy when old folder cannot be deleted (owner/permission mismatch).
            copyDirectory(extractedDataDir, targetDataDir);
            deleteRecursivelyQuietly(extractedDataDir);
        }
        List<String> missingEntries = findMissingTakumiDataEntries(targetDataDir);
        if (!missingEntries.isEmpty()) {
            throw new IOException("downloaded Takumi data incomplete, missing: "
                + joinMissingEntries(missingEntries));
        }
        writeDataReadyMarker(externalRoot, targetDataDir);

        postUi(() -> progressBar.setProgress(1000));
    }

    private boolean tryDeleteExistingDataDir(File dir) {
        try {
            deleteRecursively(dir);
            return true;
        } catch (IOException deleteError) {
            Log.w(TAG, "Unable to clean existing data dir, fallback to in-place overwrite: "
                + dir.getAbsolutePath() + " (" + deleteError.getMessage() + ")");
            return false;
        }
    }

    private static void deleteRecursivelyQuietly(File file) {
        try {
            deleteRecursively(file);
        } catch (IOException ignored) {
            // best effort cleanup for temporary extraction folder
        }
    }

    private boolean moveDirectory(File sourceDir, File targetDir) {
        if (sourceDir == null || targetDir == null) {
            return false;
        }
        File parent = targetDir.getParentFile();
        if (parent != null && !parent.exists() && !parent.mkdirs()) {
            return false;
        }
        return sourceDir.renameTo(targetDir);
    }

    private void copyDirectory(File sourceDir, File targetDir) throws IOException {
        ensureDirectory(targetDir);
        File[] children = sourceDir.listFiles();
        if (children == null) {
            return;
        }
        for (File child : children) {
            checkCancelled();
            File targetChild = resolvePathCaseInsensitive(targetDir, child.getName(), child.isDirectory());
            if (child.isDirectory()) {
                copyDirectory(child, targetChild);
            } else {
                copyFile(child, targetChild);
            }
        }
    }

    private void copyFile(File sourceFile, File targetFile) throws IOException {
        File parent = targetFile.getParentFile();
        if (parent != null) {
            ensureDirectory(parent);
        }
        try (InputStream input = new BufferedInputStream(new FileInputStream(sourceFile), BUFFER_SIZE);
             OutputStream output = new BufferedOutputStream(new FileOutputStream(targetFile, false), BUFFER_SIZE)) {
            byte[] buffer = new byte[BUFFER_SIZE];
            int read;
            while ((read = input.read(buffer)) != -1) {
                checkCancelled();
                output.write(buffer, 0, read);
            }
            output.flush();
        }
    }

    private static File findChildDirectoryIgnoreCase(File parent, String childName) {
        if (parent == null || childName == null) {
            return null;
        }
        File[] children = parent.listFiles();
        if (children == null) {
            return null;
        }
        for (File child : children) {
            if (child.isDirectory() && child.getName().equalsIgnoreCase(childName)) {
                return child;
            }
        }
        return null;
    }

    private boolean shouldSkipDownload(File existingDataDir, File externalRoot) {
        File marker = new File(externalRoot, DATA_READY_MARKER_FILE);
        if (isDataFolderUsable(existingDataDir)) {
            writeDataReadyMarker(externalRoot, existingDataDir);
            Log.i(TAG, "Skip download, usable data exists: " + existingDataDir.getAbsolutePath()
                + " markerWasPresent=" + marker.exists());
            return true;
        }

        Log.i(TAG, "Data not usable, download required. root=" + externalRoot.getAbsolutePath()
            + " marker=" + marker.exists());
        return false;
    }

    private boolean isDataFolderUsable(File dataDir) {
        if (dataDir == null || !dataDir.exists() || !dataDir.isDirectory()) {
            return false;
        }

        List<String> missingEntries = findMissingTakumiDataEntries(dataDir);
        if (!missingEntries.isEmpty()) {
            Log.i(TAG, "Takumi data incomplete at " + dataDir.getAbsolutePath()
                + ", missing: " + joinMissingEntries(missingEntries));
            return false;
        }

        int hintMatches = 0;
        for (String hint : DATA_DIR_HINTS) {
            File child = findChildDirectoryIgnoreCase(dataDir, hint);
            if (child != null && child.isDirectory()) {
                hintMatches++;
            }
        }
        if (hintMatches >= 2) {
            return true;
        }

        DataScanResult scanResult = quickScanDataFolder(dataDir, 3, 64);
        return scanResult.fileCount >= 10 && scanResult.totalBytes >= (256L * 1024L);
    }

    private static List<String> findMissingTakumiDataEntries(File dataDir) {
        List<String> missingEntries = new ArrayList<>();

        for (String requiredDir : TAKUMI_REQUIRED_DIRS) {
            File candidate = findPathIgnoreCase(dataDir, requiredDir);
            if (candidate == null || !candidate.isDirectory()) {
                missingEntries.add(requiredDir + "/");
            }
        }

        for (String requiredFile : TAKUMI_REQUIRED_FILES) {
            File candidate = findPathIgnoreCase(dataDir, requiredFile);
            if (candidate == null || !candidate.isFile() || candidate.length() <= 0L) {
                missingEntries.add(requiredFile);
            }
        }

        return missingEntries;
    }

    private static File findPathIgnoreCase(File rootDir, String relativePath) {
        String normalizedPath = normalizeZipPath(relativePath);
        if (rootDir == null || normalizedPath.isEmpty()) {
            return null;
        }

        File current = rootDir;
        String[] segments = normalizedPath.split("/");
        for (String segment : segments) {
            current = findChildIgnoreCase(current, segment);
            if (current == null) {
                return null;
            }
        }
        return current;
    }

    private static String joinMissingEntries(List<String> missingEntries) {
        if (missingEntries == null || missingEntries.isEmpty()) {
            return "";
        }

        int limit = Math.min(8, missingEntries.size());
        StringBuilder builder = new StringBuilder();
        for (int i = 0; i < limit; i++) {
            if (i > 0) {
                builder.append(", ");
            }
            builder.append(missingEntries.get(i));
        }
        if (missingEntries.size() > limit) {
            builder.append(" ... +").append(missingEntries.size() - limit).append(" more");
        }
        return builder.toString();
    }

    private static DataScanResult quickScanDataFolder(File dir, int maxDepth, int fileLimit) {
        DataScanResult result = new DataScanResult();
        quickScanRecursive(dir, 0, maxDepth, fileLimit, result);
        return result;
    }

    private static void quickScanRecursive(
        File dir,
        int depth,
        int maxDepth,
        int fileLimit,
        DataScanResult result) {

        if (dir == null || result.fileCount >= fileLimit) {
            return;
        }

        File[] children = dir.listFiles();
        if (children == null) {
            return;
        }

        for (File child : children) {
            if (result.fileCount >= fileLimit) {
                break;
            }

            if (child.isFile()) {
                long len = child.length();
                if (len > 0L) {
                    result.fileCount++;
                    result.totalBytes += len;
                }
                continue;
            }

            if (child.isDirectory() && depth < maxDepth) {
                quickScanRecursive(child, depth + 1, maxDepth, fileLimit, result);
            }
        }
    }

    private void writeDataReadyMarker(File externalRoot, File dataDir) {
        if (externalRoot == null || dataDir == null) {
            return;
        }

        File marker = new File(externalRoot, DATA_READY_MARKER_FILE);
        String payload = "ready_at=" + System.currentTimeMillis()
            + "\npath=" + dataDir.getAbsolutePath() + "\n";
        try (OutputStream output = new FileOutputStream(marker, false)) {
            output.write(payload.getBytes(StandardCharsets.UTF_8));
            output.flush();
        } catch (IOException markerError) {
            Log.w(TAG, "Failed to write data marker: " + markerError.getMessage());
        }
    }

    private static String normalizeZipPath(String entryName) {
        if (entryName == null) {
            return "";
        }
        String path = entryName.replace('\\', '/');
        while (path.startsWith("/")) {
            path = path.substring(1);
        }

        String[] rawParts = path.split("/");
        List<String> safeParts = new ArrayList<>(rawParts.length);
        for (String part : rawParts) {
            if (part.isEmpty() || ".".equals(part)) {
                continue;
            }
            if ("..".equals(part)) {
                return "";
            }
            safeParts.add(part);
        }
        if (safeParts.isEmpty()) {
            return "";
        }
        StringBuilder builder = new StringBuilder();
        for (int i = 0; i < safeParts.size(); i++) {
            if (i > 0) {
                builder.append('/');
            }
            builder.append(safeParts.get(i));
        }
        return builder.toString();
    }

    private static File resolvePathCaseInsensitive(File rootDir, String relativePath, boolean asDirectory) throws IOException {
        String normalizedPath = normalizeZipPath(relativePath);
        if (normalizedPath.isEmpty()) {
            return rootDir;
        }

        String[] segments = normalizedPath.split("/");
        File current = rootDir;

        for (int i = 0; i < segments.length; i++) {
            String segment = segments[i];
            boolean last = i == segments.length - 1;
            File existing = findChildIgnoreCase(current, segment);
            File resolved = existing != null ? existing : new File(current, segment);

            if (!last || asDirectory) {
                ensureDirectory(resolved);
            } else {
                File parent = resolved.getParentFile();
                if (parent != null) {
                    ensureDirectory(parent);
                }
            }

            current = resolved;
        }

        return current;
    }

    private static File findChildIgnoreCase(File parent, String childName) {
        if (parent == null || childName == null) {
            return null;
        }
        File[] children = parent.listFiles();
        if (children == null) {
            return null;
        }
        for (File child : children) {
            if (child.getName().equalsIgnoreCase(childName)) {
                return child;
            }
        }
        return null;
    }

    private static void assertUnderRoot(File root, File candidate) throws IOException {
        File canonicalRoot = root.getCanonicalFile();
        File canonicalCandidate = candidate.getCanonicalFile();
        String rootPath = canonicalRoot.getPath();
        String candidatePath = canonicalCandidate.getPath();
        if (!candidatePath.equals(rootPath) && !candidatePath.startsWith(rootPath + File.separator)) {
            throw new IOException("Invalid zip entry path: " + candidatePath);
        }
    }

    private static void recreateDirectory(File dir) throws IOException {
        deleteRecursively(dir);
        ensureDirectory(dir);
    }

    private static void ensureDirectory(File dir) throws IOException {
        if (dir == null) {
            throw new IOException("Directory is null");
        }
        if (dir.exists()) {
            if (!dir.isDirectory()) {
                throw new IOException("Path is not a directory: " + dir.getAbsolutePath());
            }
            return;
        }
        if (!dir.mkdirs() && !dir.isDirectory()) {
            throw new IOException("Failed to create directory: " + dir.getAbsolutePath());
        }
    }

    private static void deleteRecursively(File file) throws IOException {
        if (file == null || !file.exists()) {
            return;
        }
        if (file.isDirectory()) {
            File[] children = file.listFiles();
            if (children != null) {
                for (File child : children) {
                    deleteRecursively(child);
                }
            }
        }
        if (!file.delete() && file.exists()) {
            throw new IOException("Unable to delete: " + file.getAbsolutePath());
        }
    }

    private void checkCancelled() throws IOException {
        if (cancelled || Thread.currentThread().isInterrupted()) {
            throw new IOException("Preload cancelled");
        }
    }

    private double computeParseProgress(long extractedBytes, int extractedFiles, ZipMetrics metrics) {
        if (metrics.totalBytes > 0L) {
            return Math.min(1.0, extractedBytes / (double) metrics.totalBytes);
        }
        if (metrics.fileCount > 0) {
            return Math.min(1.0, extractedFiles / (double) metrics.fileCount);
        }
        return 1.0;
    }

    private void updateDownloadUi(long downloadedBytes, long totalBytes, long elapsedMs, double overallPercent) {
        double speedBytesPerSecond = downloadedBytes / Math.max(0.001, elapsedMs / 1000.0);
        long etaMs = 0L;
        if (totalBytes > 0 && speedBytesPerSecond > 0.0) {
            etaMs = (long) (((double) (totalBytes - downloadedBytes) / speedBytesPerSecond) * 1000.0);
            if (etaMs < 0L) {
                etaMs = 0L;
            }
        }

        final String stage = totalBytes > 0
            ? String.format(Locale.US, "Downloading data.zip %.2f%%",
                (downloadedBytes * 100.0) / Math.max(1L, totalBytes))
            : "Downloading data.zip";
        final String detail = totalBytes > 0
            ? "Downloaded " + formatBytes(downloadedBytes) + " / " + formatBytes(totalBytes)
            : "Downloaded " + formatBytes(downloadedBytes);
        final String timer = "Speed " + formatBytes((long) speedBytesPerSecond) + "/s"
            + " | ETA " + (totalBytes > 0 ? formatDuration(etaMs) : "--:--");

        updateStageUi(stage, detail, timer, overallPercent);
    }

    private void updateExtractUi(
        long extractedBytes,
        int extractedFiles,
        ZipMetrics metrics,
        long elapsedMs,
        double overallPercent) {

        double speedBytesPerSecond = extractedBytes / Math.max(0.001, elapsedMs / 1000.0);
        long etaMs = 0L;
        if (metrics.totalBytes > 0 && speedBytesPerSecond > 0.0) {
            etaMs = (long) (((double) (metrics.totalBytes - extractedBytes) / speedBytesPerSecond) * 1000.0);
            if (etaMs < 0L) {
                etaMs = 0L;
            }
        }

        double stagePercent = metrics.totalBytes > 0
            ? (extractedBytes * 100.0) / Math.max(1L, metrics.totalBytes)
            : (metrics.fileCount > 0 ? (extractedFiles * 100.0) / metrics.fileCount : 100.0);
        final String stage = String.format(Locale.US, "Parsing data folder %.2f%%", stagePercent);
        final String detail = metrics.totalBytes > 0
            ? "Parsed " + formatBytes(extractedBytes) + " / " + formatBytes(metrics.totalBytes)
            : "Parsed files " + extractedFiles + " / " + metrics.fileCount;
        final String timer = metrics.totalBytes > 0
            ? ("Speed " + formatBytes((long) speedBytesPerSecond) + "/s | ETA " + formatDuration(etaMs))
            : ("Elapsed " + formatDuration(elapsedMs));

        updateStageUi(stage, detail, timer, overallPercent);
    }

    private void updateInstallUi(String stage) {
        updateStageUi(stage, "Applying into device old data location...", "Finalizing...", 99.0);
    }

    private void updateStageUi(String stage, String detail, String timer, double overallPercent) {
        final int progressValue = toProgress(overallPercent);
        postUi(() -> {
            if (cancelled) {
                return;
            }
            stageText.setText(stage);
            detailText.setText(detail);
            timerText.setText(timer);
            progressBar.setProgress(progressValue);
        });
    }

    private int toProgress(double percent) {
        double safe = Math.max(0.0, Math.min(100.0, percent));
        return (int) Math.round(safe * 10.0);
    }

    private void postUi(Runnable uiWork) {
        mainHandler.post(uiWork);
    }

    private void launchGame() {
        if (cancelled) {
            return;
        }
        Intent intent = new Intent(this, MuMainNativeActivity.class);
        intent.addFlags(Intent.FLAG_ACTIVITY_CLEAR_TOP | Intent.FLAG_ACTIVITY_NEW_TASK);
        startActivity(intent);
        finish();
    }

    private static String formatDuration(long millis) {
        long totalSeconds = Math.max(0L, millis / 1000L);
        long hours = totalSeconds / 3600L;
        long minutes = (totalSeconds % 3600L) / 60L;
        long seconds = totalSeconds % 60L;

        if (hours > 0) {
            return String.format(Locale.US, "%d:%02d:%02d", hours, minutes, seconds);
        }
        return String.format(Locale.US, "%02d:%02d", minutes, seconds);
    }

    private static String formatBytes(long bytes) {
        if (bytes < 1024L) {
            return bytes + " B";
        }

        final String[] units = {"KB", "MB", "GB", "TB"};
        double value = bytes / 1024.0;
        int unitIndex = 0;
        while (value >= 1024.0 && unitIndex < units.length - 1) {
            value /= 1024.0;
            unitIndex++;
        }
        return String.format(Locale.US, "%.2f %s", value, units[unitIndex]);
    }

    private static final class DownloadStats {
        final long downloadedBytes;
        final long totalBytes;
        final long durationMs;

        DownloadStats(long downloadedBytes, long totalBytes, long durationMs) {
            this.downloadedBytes = downloadedBytes;
            this.totalBytes = totalBytes;
            this.durationMs = durationMs;
        }
    }

    private static final class ZipMetrics {
        final long totalBytes;
        final int fileCount;

        ZipMetrics(long totalBytes, int fileCount) {
            this.totalBytes = totalBytes;
            this.fileCount = fileCount;
        }
    }

    private static final class DataScanResult {
        int fileCount;
        long totalBytes;
    }

    private static final class HttpStatusException extends IOException {
        final int statusCode;

        HttpStatusException(int statusCode, String message) {
            super(message);
            this.statusCode = statusCode;
        }
    }
}
