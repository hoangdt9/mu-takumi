@echo off
title Start Synced MuServer - 192.168.99.200
color 0A
set "MUSERVER=D:\takumi\MuServer"

echo Killing old server processes...
taskkill /F /IM ConnectServer.exe >nul 2>nul
taskkill /F /IM DataServer.exe >nul 2>nul
taskkill /F /IM JoinServer.exe >nul 2>nul
taskkill /F /IM GameServer.exe >nul 2>nul
taskkill /F /IM XShield.exe >nul 2>nul
timeout /t 2 /nobreak >nul

echo [1/5] ConnectServer
start "ConnectServer" /D "%MUSERVER%\1.ConnectServer" "%MUSERVER%\1.ConnectServer\ConnectServer.exe"
timeout /t 3 /nobreak >nul

echo [2/5] DataServer
start "DataServer" /D "%MUSERVER%\2.DataServer" "%MUSERVER%\2.DataServer\DataServer.exe"
timeout /t 3 /nobreak >nul

echo [3/5] JoinServer
start "JoinServer" /D "%MUSERVER%\3.JoinServer" "%MUSERVER%\3.JoinServer\JoinServer.exe"
timeout /t 3 /nobreak >nul

echo [4/5] XShield
start "XShield" /D "%MUSERVER%\5.Antihack" "%MUSERVER%\5.Antihack\XShield.exe"
timeout /t 3 /nobreak >nul

echo [5/5] GameServer
start "GameServer" /D "%MUSERVER%\4.GameServer\Sub 1\GameServer" "%MUSERVER%\4.GameServer\Sub 1\GameServer\GameServer.exe"

echo Done. Use client: D:\takumi\ClientBuild_192.168.99.200\StartClient.bat
pause
