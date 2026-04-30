@echo off
title Start MuServer - 192.168.99.200
color 0A

echo =====================================
echo        STARTING MU SERVER
echo        IP: 192.168.99.200
echo =====================================
echo.

cd /d "%~dp0"

echo [1/5] Starting ConnectServer...
start "ConnectServer" "1.ConnectServer\ConnectServer.exe"
timeout /t 3 /nobreak >nul

echo [2/5] Starting DataServer...
start "DataServer" "2.DataServer\DataServer.exe"
timeout /t 3 /nobreak >nul

echo [3/5] Starting JoinServer...
start "JoinServer" "3.JoinServer\JoinServer.exe"
timeout /t 3 /nobreak >nul

echo [4/5] Starting AntiHack XShield...
start "XShield" "5.Antihack\XShield.exe"
timeout /t 3 /nobreak >nul

echo [5/5] Starting GameServer...
start "GameServer" "4.GameServer\Sub 1\GameServer\GameServer.exe"

echo.
echo =====================================
echo Done. Check each server window for errors.
echo Neu loi DB thi kiem tra ODBC MuOnline 32-bit va SQL dang chay.
echo =====================================
pause
