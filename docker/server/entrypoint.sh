#!/bin/bash
set -euo pipefail

export DISPLAY="${DISPLAY:-:99}"
export WINEARCH="${WINEARCH:-win32}"
export WINEDEBUG="${WINEDEBUG:--all}"

mkdir -p "${WINEPREFIX:?}"

if [[ ! -d /MuServer ]]; then
	echo "Missing /MuServer mount. Set MU_SERVER_HOST_PATH to your MuServer folder." >&2
	exit 1
fi

rm -rf /tmp/.X99-lock 2>/dev/null || true
Xvfb "${DISPLAY}" -screen 0 1024x768x16 -nolisten tcp &
sleep 1

between() {
	sleep "${START_DELAY_SECONDS:-3}"
}

start_one() {
	local dir="$1"
	local exe="$2"
	local path="${dir}/${exe}"
	if [[ -f "$path" ]]; then
		echo "[entrypoint] starting $path"
		(cd "$dir" && wine "./$exe") &
		between || true
		return 0
	fi
	return 1
}

started=0

# Giống Start_192.168.99.200.bat (Connect → Data → Join → XShield? → GSCS → GS thứ 2)
start_one "/MuServer/1.ConnectServer" "ConnectServer.exe" && started=$((started + 1)) || true
start_one "/MuServer/2.DataServer" "DataServer.exe" && started=$((started + 1)) || true
start_one "/MuServer/3.JoinServer" "JoinServer.exe" && started=$((started + 1)) || true

if [[ "${START_XSHIELD:-0}" == "1" ]] && [[ -f /MuServer/5.Antihack/XShield.exe ]]; then
	echo "[entrypoint] starting XShield"
	(cd "/MuServer/5.Antihack" && wine ./XShield.exe) &
	between || true
	started=$((started + 1))
fi

if [[ -n "${GAME_SERVER_EXE_OVERRIDE:-}" ]]; then
	game_dir="${GAME_SERVER_DIR_OVERRIDE:-/MuServer/4.GameServer/GameServer}"
	start_one "$game_dir" "${GAME_SERVER_EXE_OVERRIDE}" && started=$((started + 1)) || true
else
	start_one "/MuServer/4.GameServer/GameServer" "GameServerCS.exe" && started=$((started + 1)) || true
fi

if [[ "${START_SECOND_GAMESERVER:-1}" != "0" ]]; then
	start_one "/MuServer/4.GameServer/Sub 1/GameServer" "GameServer.exe" && started=$((started + 1)) || true
fi

if [[ "$started" -eq 0 ]]; then
	echo "No server .exe found under /MuServer." >&2
	ls -laR /MuServer 2>/dev/null | head -200 || true
	exit 2
fi

echo "[entrypoint] $started wine process(es); docker logs will show wine stderr. Blocking on wait."
wait
