# Android logcat — TakumiMu QA

## Script có sẵn

Từ `server-next/`:

```bash
./scripts/android/watch-android-takumi-log.sh
```

## Lệnh thủ công

```bash
adb logcat -c
adb logcat -v time | grep -E 'Takumi|ReceiveCreateMonsterViewport|0x13|0x11|0x16|0x14|MU_BOOTSTRAP|Connect'
```

## Mốc log theo milestone

| Milestone | Tìm trên logcat |
|-----------|-----------------|
| Connect | bootstrap `44605`, server list |
| Login | character list, select |
| M7 | HP/MP UI update (ít log opcode; xem server `0x26`/`0x27`) |
| M9 | `ReceiveCreateMonsterViewport`, damage, mob despawn |
| M10 | movement sync (tuỳ build client) |

## APK đã cài (terminal mẫu)

- Variant: `app-realDevice-preloadDefault-debug.apk`
- LAN: `192.168.1.50:44605` bootstrap, `data.zip` port `18080`

Nếu đổi server IP: sửa `server-next/.env` → **rebuild APK** → `adb install -r …`.

## Pass criteria

Logcat không flood crash native; có nhận `0x13` sau vào map khi test M9.
