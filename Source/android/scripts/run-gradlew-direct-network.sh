#!/usr/bin/env bash
# Run Gradle with common VPN/proxy env vars cleared (some shells export HTTP_PROXY from VPN helpers).
# Usage: ./scripts/run-gradlew-direct-network.sh :app:assembleRealDevicePreloadDefaultDebug ...
set -euo pipefail
cd "$(dirname "$0")/.."
unset HTTP_PROXY HTTPS_PROXY http_proxy https_proxy ALL_PROXY all_proxy FTP_PROXY ftp_proxy
exec ./gradlew "$@"
