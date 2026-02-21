@echo off
echo Compiling...
g++ -std=c++23 -O2 -Wall -Wextra -I include src/*.cpp -o build/game.exe
if %errorlevel% == 0 (
    echo Build successful!
    echo Running...
    build\game.exe
) else (
    echo Build FAILED!
)