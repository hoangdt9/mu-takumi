# Shared path helpers for scripts under scripts/<category>/.
# Usage (from scripts/docker/foo.sh):
#   # shellcheck source=../_lib/paths.sh
#   source "$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)/../_lib/paths.sh"
#   takumi_script_paths

takumi_script_paths() {
    local here
    here="$(cd "$(dirname "${BASH_SOURCE[1]:-${BASH_SOURCE[0]}}")" && pwd)"
    SCRIPT_DIR="$here"
    SCRIPTS_ROOT="$(cd "$SCRIPT_DIR/.." && pwd)"
    ROOT="$(cd "$SCRIPTS_ROOT/.." && pwd)"
}
