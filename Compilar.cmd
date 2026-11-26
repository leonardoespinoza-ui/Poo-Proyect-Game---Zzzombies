@echo off
setlocal
cd /d "%~dp0"
if not exist "C:\msys64\mingw32\bin\g++.exe" (
    echo Se requiere MinGW de 32 bits en C:\msys64\mingw32.
    exit /b 1
)
"C:\msys64\mingw32\bin\g++.exe" -std=c++17 -Wall -Wextra -pedantic -O2 -static -s -isystem vendor/raylib/include src/main.cpp src/Partida.cpp src/Pantallas.cpp src/Guardado.cpp -Lvendor/raylib/lib -lraylib -lsqlite3 -lopengl32 -lgdi32 -lwinmm -lshell32 -mwindows -o ZZZombies.exe
exit /b %errorlevel%

