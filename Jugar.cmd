@echo off
cd /d "%~dp0"
if not exist "build\refugio.exe" (
    powershell -NoProfile -ExecutionPolicy Bypass -File "%~dp0tools\compilar.ps1"
    if errorlevel 1 (
        pause
        exit /b 1
    )
)
start "" "%~dp0build\refugio.exe"
