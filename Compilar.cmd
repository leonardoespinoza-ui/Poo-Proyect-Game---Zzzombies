@echo off
cd /d "%~dp0"
"C:\msys64\mingw32\bin\g++.exe" -std=c++17 -Wall -Wextra -pedantic -O2 -static -s -Ivendor/raylib/include Clases.cpp -Lvendor/raylib/lib -lraylib -lopengl32 -lgdi32 -lwinmm -lshell32 -mwindows -o ZZZombies.exe
if errorlevel 1 (
    echo No se pudo compilar.
    pause
    exit /b 1
)
echo Compilado correctamente.
pause
