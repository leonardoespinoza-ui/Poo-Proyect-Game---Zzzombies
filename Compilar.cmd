@echo off
cd /d "%~dp0"
powershell -NoProfile -ExecutionPolicy Bypass -File "%~dp0tools\compilar.ps1"
if errorlevel 1 (
    pause
    exit /b 1
)
echo Listo. Abre Jugar.cmd para probar.
pause
