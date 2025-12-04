@echo off
echo Building ScuffedRedis...
echo.

REM Create build directory if it doesn't exist
if not exist build mkdir build
cd build

REM Configure with CMake
echo Configuring with CMake...
cmake .. -G "MinGW Makefiles" 2>nul || cmake .. 2>nul
if %errorlevel% neq 0 (
    echo Error: CMake configuration failed
    exit /b 1
)

REM Build the project
echo.
echo Building project...
cmake --build . --config Release
if %errorlevel% neq 0 (
    echo Error: Build failed
    exit /b 1
)

echo.
echo Build successful!
echo.
echo Executables:
echo   build\scuffed-redis-server.exe - Redis server
echo   build\scuffed-redis-cli.exe    - Redis client
echo.
echo To run:
echo   Server: build\scuffed-redis-server.exe
echo   Client: build\scuffed-redis-cli.exe
cd ..
