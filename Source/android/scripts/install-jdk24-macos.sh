#!/usr/bin/env bash
# Install Eclipse Temurin JDK 24 (aarch64 or x64) under ~/.jdks for Gradle 8.14.x.
# Usage: ./scripts/install-jdk24-macos.sh
set -euo pipefail

readonly VERSION_TAG="jdk-24.0.2%2B12"
readonly VERSION_DIR="jdk-24.0.2+12"
readonly BASE="https://github.com/adoptium/temurin24-binaries/releases/download/${VERSION_TAG}"

arch="$(uname -m)"
case "$arch" in
  arm64) pkg="OpenJDK24U-jdk_aarch64_mac_hotspot_24.0.2_12.tar.gz" ;;
  x86_64) pkg="OpenJDK24U-jdk_x64_mac_hotspot_24.0.2_12.tar.gz" ;;
  *)
    echo "Unsupported uname -m: $arch" >&2
    exit 1
    ;;
esac

dest_root="${HOME}/.jdks"
mkdir -p "$dest_root"
tmp="$(mktemp -t temurin24.XXXXXX.tar.gz)"
trap 'rm -f "$tmp"' EXIT

echo "Downloading ${pkg} ..."
curl -fSL --retry 3 --retry-delay 2 -o "$tmp" "${BASE}/${pkg}"

echo "Extracting to ${dest_root}/${VERSION_DIR} ..."
rm -rf "${dest_root}/${VERSION_DIR}"
tar -xzf "$tmp" -C "$dest_root"

java_home="${dest_root}/${VERSION_DIR}/Contents/Home"
"${java_home}/bin/java" -version

echo ""
echo "Installed JDK 24 at:"
echo "  ${java_home}"
echo ""
echo "Point Gradle 8.14.x at it (pick one):"
echo "  1) In Source/android/gradle.properties set:"
echo "       org.gradle.java.home=${java_home}"
echo "  2) Or export before ./gradlew:"
echo "       export JAVA_HOME=${java_home}"
echo ""
echo "Optional (IDE / /usr/libexec/java_home):"
echo "  sudo ln -sfn ${dest_root}/${VERSION_DIR} /Library/Java/JavaVirtualMachines/temurin-24.jdk"
