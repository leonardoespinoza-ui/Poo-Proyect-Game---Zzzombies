@echo off
cd /d "%~dp0"
if not exist ZZZombies.exe call Compilar.cmd
if not exist ZZZombies.exe exit /b 1
start "" "%~dp0ZZZombies.exe"
