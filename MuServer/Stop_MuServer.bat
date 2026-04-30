@echo off
title Stop MuServer
color 0C
setlocal

echo ==================================================
echo Stopping MuServer processes...
echo ==================================================

taskkill /F /IM ConnectServer.exe >nul 2>nul
taskkill /F /IM DataServer.exe >nul 2>nul
taskkill /F /IM JoinServer.exe >nul 2>nul
taskkill /F /IM GameServer.exe >nul 2>nul
taskkill /F /IM GameServerCS.exe >nul 2>nul
taskkill /F /IM XShield.exe >nul 2>nul

echo Done.
timeout /t 2 /nobreak >nul
endlocal
