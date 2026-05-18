#!/usr/bin/env bash
# Merge assets/data-patches into a ClientBuild Data/ tree and optionally repack data.zip.
set -euo pipefail

ROOT="$(cd "$(dirname "$0")/.." && pwd)"
PATCHES="${ROOT}/assets/data-patches"
REPACK_ZIP=0

for arg in "$@"; do
  case "$arg" in
    --repack-zip) REPACK_ZIP=1 ;;
    -h|--help)
      echo "Usage: $0 [--repack-zip]"
      echo "  Copies assets/data-patches/World* into ClientBuild Data/ and optionally rebuilds data.zip."
      exit 0
      ;;
    *) echo "Unknown option: $arg" >&2; exit 2 ;;
  esac
done

pick_client_build() {
  local d
  for d in "${ROOT}"/ClientBuild_*/Data; do
    if [[ -d "$d" ]]; then
      echo "$(dirname "$d")"
      return 0
    fi
  done
  for d in "${ROOT}"/ClientBuild/Data; do
    if [[ -d "$d" ]]; then
      echo "${ROOT}/ClientBuild"
      return 0
    fi
  done
  echo "No ClientBuild*/Data found under ${ROOT}" >&2
  exit 1
}

CB="$(pick_client_build)"
DATA="${CB}/Data"

if [[ ! -d "$PATCHES" ]]; then
  echo "Missing ${PATCHES}" >&2
  exit 1
fi

echo "[apply-data-patches] target Data: ${DATA}"
for world_dir in "${PATCHES}"/World*; do
  [[ -d "$world_dir" ]] || continue
  world="$(basename "$world_dir")"
  mkdir -p "${DATA}/${world}"
  cp -f "${world_dir}/"* "${DATA}/${world}/" 2>/dev/null || true
  echo "  ${world}: $(ls -1 "${world_dir}" | tr '\n' ' ')"
done

if [[ "$REPACK_ZIP" -eq 1 ]]; then
  ZIP="${CB}/data.zip"
  echo "[apply-data-patches] repack ${ZIP}"
  (cd "${CB}" && zip -qr data.zip Data -x "*.DS_Store")
  for host_dir in \
    "${ROOT}/docker/data-zip/host" \
    "${ROOT}/server-next/docker/data-zip/host"; do
    if [[ -d "$(dirname "$host_dir")" ]]; then
      mkdir -p "$host_dir"
      cp -f "$ZIP" "${host_dir}/data.zip"
      echo "[apply-data-patches] copied → ${host_dir}/data.zip"
    fi
  done
fi

echo "[apply-data-patches] done."
