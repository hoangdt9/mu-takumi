#!/usr/bin/env bash
# Remove macOS system JDKs 17/21 and register only Temurin JDK 24 from ~/.jdks/jdk-24.0.2+12.
# Requires: interactive Terminal (sudo + brew cask uninstall passwords).
set -euo pipefail

readonly JDK24="${HOME}/.jdks/jdk-24.0.2+12"
if [[ ! -x "${JDK24}/Contents/Home/bin/java" ]]; then
  echo "Missing JDK 24 at ${JDK24}. Run: ./scripts/install-jdk24-macos.sh" >&2
  exit 1
fi

echo "== Homebrew: remove Zulu 17 cask (will ask for password) =="
if /opt/homebrew/bin/brew list --cask 2>/dev/null | grep -q '^zulu@17$'; then
  /opt/homebrew/bin/brew uninstall --cask zulu@17 || true
fi
if /opt/homebrew/bin/brew list --cask 2>/dev/null | grep -q '^zulu17$'; then
  /opt/homebrew/bin/brew uninstall --cask zulu17 || true
fi

for f in zulu17 zulu@17; do
  if /opt/homebrew/bin/brew list --formula 2>/dev/null | grep -qx "$f"; then
    /opt/homebrew/bin/brew uninstall --force "$f" || true
  fi
done

echo "== /Library/Java/JavaVirtualMachines: remove Temurin 21 + Zulu 17 =="
sudo rm -rf \
  /Library/Java/JavaVirtualMachines/temurin-21.jdk \
  /Library/Java/JavaVirtualMachines/zulu-17.jdk

echo "== Register JDK 24 for java_home / IDEs =="
sudo ln -sfn "${JDK24}" /Library/Java/JavaVirtualMachines/temurin-24.jdk

echo ""
"${JDK24}/Contents/Home/bin/java" -version
echo ""
/usr/libexec/java_home -V 2>&1 || true
echo ""
echo "Done. Set shell default (add to ~/.zshrc if you want):"
echo "  export JAVA_HOME=\"${JDK24}/Contents/Home\""
echo "  export PATH=\"\$JAVA_HOME/bin:\$PATH\""
echo ""
echo "Note: Homebrew maven/bfg may report missing openjdk; use JAVA_HOME above or: brew install openjdk"
