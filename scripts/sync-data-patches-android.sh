#!/usr/bin/env bash
# Push assets/data-patches (Monster BMD, World leaf, …) to Android app external Data/.
# Use when DEV_SKIP_DATA_ZIP skips HTTP data.zip but phone still has stale S20 assets.
set -euo pipefail

ROOT="$(cd "$(dirname "$0")/.." && pwd)"
PATCHES="${ROOT}/assets/data-patches"
PKG="${MU_ANDROID_PACKAGE:-com.muonline.client}"
REMOTE_BASE="${MU_ANDROID_DATA_BASE:-/storage/emulated/0/Android/data/${PKG}/files}"
REMOTE_DATA="${REMOTE_BASE}/Data"
APPLY_LOCAL=0
DRY_RUN=0
MONSTER_ONLY=1
WORLDS=0

usage() {
  cat <<'EOF'
Usage: sync-data-patches-android.sh [options]

  Pushes files from assets/data-patches/ onto the device:
    Data/Monster/*.bmd
    Data/World*/…  (only with --worlds)

Options:
  --apply-local     Run ./scripts/apply-data-patches.sh first (updates ClientBuild)
  --worlds          Also push World* patches (leaf OZT/OZJ), not only Monster/
  --all             Push Monster + World* (same as --worlds)
  --package PKG     Android applicationId (default: com.muonline.client)
  --remote-base P   Parent of Data/ on device (default: .../Android/data/<pkg>/files)
  --dry-run         Print adb commands without executing
  -h, --help        Show this help

Examples:
  ./scripts/sync-data-patches-android.sh
  ./scripts/sync-data-patches-android.sh --apply-local
  ./scripts/sync-data-patches-android.sh --worlds
  ./scripts/sync-data-patches-android.sh --dry-run

After push: force-quit the app and relaunch (models load at startup).
Verify sizes on device:
  adb shell ls -la .../files/Data/Monster/Monster03.bmd Monster04.bmd

Expected classic S6 sizes (MuMain-5.2 reference):
  Monster03.bmd  ~87994 bytes  (Budge Dragon, class 2)
  Monster04.bmd  ~166065 bytes (Spider, class 3)

Alternative (full bundle): ./scripts/apply-data-patches.sh --repack-zip
  then wipe app storage or install datafresh APK to re-download data.zip.
EOF
}

while [[ $# -gt 0 ]]; do
  case "$1" in
    --apply-local) APPLY_LOCAL=1; shift ;;
    --worlds) MONSTER_ONLY=0; WORLDS=1; shift ;;
    --all) MONSTER_ONLY=0; WORLDS=1; shift ;;
    --package)
      shift
      PKG="${1:?--package requires value}"
      REMOTE_BASE="${MU_ANDROID_DATA_BASE:-/storage/emulated/0/Android/data/${PKG}/files}"
      REMOTE_DATA="${REMOTE_BASE}/Data"
      shift
      ;;
    --remote-base)
      shift
      REMOTE_BASE="${1:?--remote-base requires value}"
      REMOTE_DATA="${REMOTE_BASE}/Data"
      shift
      ;;
    --dry-run) DRY_RUN=1; shift ;;
    -h|--help) usage; exit 0 ;;
    *)
      echo "Unknown option: $1" >&2
      usage >&2
      exit 2
      ;;
  esac
done

if [[ ! -d "$PATCHES" ]]; then
  echo "Missing ${PATCHES}" >&2
  exit 1
fi

if ! command -v adb >/dev/null 2>&1; then
  echo "adb not found in PATH" >&2
  exit 1
fi

device_count="$(adb devices | awk 'NR>1 && $2=="device" { c++ } END { print c+0 }')"
if [[ "$device_count" -lt 1 ]]; then
  echo "No adb device in 'device' state. Plug in USB or start emulator." >&2
  exit 1
fi

if [[ "$APPLY_LOCAL" -eq 1 ]]; then
  echo "[sync-android] apply-data-patches (ClientBuild)…"
  if [[ "$DRY_RUN" -eq 0 ]]; then
    "${ROOT}/scripts/apply-data-patches.sh"
  else
    echo "  (dry-run) ${ROOT}/scripts/apply-data-patches.sh"
  fi
fi

adb_mkdir() {
  local dir="$1"
  if [[ "$DRY_RUN" -eq 1 ]]; then
    echo "  adb shell mkdir -p ${dir}"
  else
    adb shell mkdir -p "$dir"
  fi
}

adb_push() {
  local src="$1"
  local dst="$2"
  if [[ "$DRY_RUN" -eq 1 ]]; then
    echo "  adb push ${src} ${dst}"
  else
    adb push "$src" "$dst"
  fi
}

echo "[sync-android] package=${PKG}"
echo "[sync-android] remote Data=${REMOTE_DATA}"

files_to_push=()

if [[ "$MONSTER_ONLY" -eq 0 || -d "${PATCHES}/Monster" ]]; then
  monster_dir="${PATCHES}/Monster"
  if [[ -d "$monster_dir" ]]; then
    adb_mkdir "${REMOTE_DATA}/Monster"
    while IFS= read -r -d '' f; do
      files_to_push+=("$f")
    done < <(find "$monster_dir" -maxdepth 1 -type f -print0)
  fi
fi

if [[ "$WORLDS" -eq 1 ]]; then
  for world_dir in "${PATCHES}"/World*; do
    [[ -d "$world_dir" ]] || continue
    world="$(basename "$world_dir")"
    adb_mkdir "${REMOTE_DATA}/${world}"
    while IFS= read -r -d '' f; do
      files_to_push+=("$f")
    done < <(find "$world_dir" -maxdepth 1 -type f -print0)
  done
fi

if [[ "${#files_to_push[@]}" -eq 0 ]]; then
  echo "No patch files to push under ${PATCHES}" >&2
  exit 1
fi

for src in "${files_to_push[@]}"; do
  rel="${src#${PATCHES}/}"
  dst="${REMOTE_DATA}/${rel}"
  echo "  push ${rel}"
  adb_push "$src" "$dst"
done

echo "[sync-android] done (${#files_to_push[@]} file(s))."
echo "[sync-android] Restart the game app to reload BMD/OZT."

if [[ -f "${PATCHES}/Monster/Monster03.bmd" && "$DRY_RUN" -eq 0 ]]; then
  echo "[sync-android] verify (optional):"
  adb shell ls -la "${REMOTE_DATA}/Monster/Monster03.bmd" "${REMOTE_DATA}/Monster/Monster04.bmd" 2>/dev/null \
    || adb shell ls -la "${REMOTE_DATA}/Monster/" 2>/dev/null \
    || true
fi
