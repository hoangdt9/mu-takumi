# Host folder for `data.zip`

1. Copy your client **`data.zip`** here as **`data.zip`** (exact name).
2. Start the static server from `takumi/docker`:

   ```bash
   docker compose --profile datazip up -d
   ```

3. On your phone/emulator, use the URL **`http://<LAN-IP-of-this-machine>:18080/data.zip`**  
   (default port `18080`; override with `DATA_ZIP_PUBLISH_PORT` in `docker/.env`).

The Android app tries **`BuildConfig.DATA_ZIP_URL_LAN`** first (see `app/build.gradle`, property `-PmuDataZipLan=...`), then the public fallback URL.
