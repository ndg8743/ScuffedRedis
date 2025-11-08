FROM ubuntu:22.04

# Install dependencies
RUN apt-get update && apt-get install -y \
    build-essential \
    cmake \
    g++ \
    && rm -rf /var/lib/apt/lists/*

# Set working directory
WORKDIR /app

# Copy source code
COPY src/ ./src/
COPY CMakeLists.txt ./

# Build the application
RUN mkdir -p build && cd build && \
    cmake .. && \
    make

# Expose the Redis port
EXPOSE 6379

# Run the server (binary name from CMake target)
CMD ["./build/redis-server", "6379", "0.0.0.0"]
