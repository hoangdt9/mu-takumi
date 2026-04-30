@echo off
title Start Synced MuServer - 192.168.99.200
color 0A
setlocal

set "MUSERVER=D:\takumi\MuServer"
set "RUN_XSHIELD=1"
set "RUN_GSCS=1"

echo ==================================================
echo Starting MuServer on 192.168.99.200
echo Root: %MUSERVER%
echo ==================================================
echo.

call "%MUSERVER%\Stop_MuServer.bat" >nul 2>nul
timeout /t 2 /nobreak >nul

echo [1/6] ConnectServer
start "ConnectServer" /D "%MUSERVER%\1.ConnectServer" "%MUSERVER%\1.ConnectServer\ConnectServer.exe"
timeout /t 3 /nobreak >nul

echo [2/6] DataServer
start "DataServer" /D "%MUSERVER%\2.DataServer" "%MUSERVER%\2.DataServer\DataServer.exe"
timeout /t 3 /nobreak >nul

echo [3/6] JoinServer
start "JoinServer" /D "%MUSERVER%\3.JoinServer" "%MUSERVER%\3.JoinServer\JoinServer.exe"
timeout /t 3 /nobreak >nul

if "%RUN_XSHIELD%"=="1" (
	echo [4/6] XShield
	start "XShield" /D "%MUSERVER%\5.Antihack" "%MUSERVER%\5.Antihack\XShield.exe"
	timeout /t 3 /nobreak >nul
) else (
	echo [4/6] XShield skipped
)

if "%RUN_GSCS%"=="1" (
	if not exist "%MUSERVER%\4.GameServer\GameServer\GameServerCS.exe" (
		echo [ERROR] Missing %MUSERVER%\4.GameServer\GameServer\GameServerCS.exe
		pause
		exit /b 1
	)
	if not exist "%MUSERVER%\4.GameServer\GameServer\Data\GameServerInfo - Common.ini" (
		echo [ERROR] Missing GSCS runtime config
		echo [ERROR] Expected: %MUSERVER%\4.GameServer\GameServer\Data\GameServerInfo - Common.ini
		pause
		exit /b 1
	)
	echo [5/6] GameServerCS
	start "GameServerCS" /D "%MUSERVER%\4.GameServer\GameServer" "%MUSERVER%\4.GameServer\GameServer\GameServerCS.exe"
	timeout /t 3 /nobreak >nul
)

echo [6/6] GameServer
start "GameServer" /D "%MUSERVER%\4.GameServer\Sub 1\GameServer" "%MUSERVER%\4.GameServer\Sub 1\GameServer\GameServer.exe"
timeout /t 3 /nobreak >nul

echo.
echo Done.
echo Client: D:\takumi\ClientBuild_192.168.99.200\StartClient.bat
pause
endlocal
