#!/bin/bash

echo "Building ScuffedRedis..."
echo

# Create build directory if it doesn't exist
if [ ! -d "build" ]; then
    mkdir build
fi

cd build

# Configure with CMake
echo "Configuring with CMake..."
cmake ..
if [ $? -ne 0 ]; then
    echo "Error: CMake configuration failed"
    exit 1
fi

# Build the project
echo
echo "Building project..."
cmake --build . --config Release -- -j$(nproc)
if [ $? -ne 0 ]; then
    echo "Error: Build failed"
    exit 1
fi

echo
echo "Build successful!"
echo
echo "Executables:"
echo "  ./build/scuffed-redis-server - Redis server"
echo "  ./build/scuffed-redis-cli     - Redis client"
echo
echo "To run:"
echo "  Server: ./build/scuffed-redis-server"
echo "  Client: ./build/scuffed-redis-cli"

cd ..
