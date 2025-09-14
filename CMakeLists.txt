cmake_minimum_required(VERSION 3.16)
project(ScuffedRedis VERSION 1.0.0 LANGUAGES CXX)

# Set C++ standard
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Compiler flags for optimization and debugging
set(CMAKE_CXX_FLAGS_DEBUG "-g -O0 -Wall -Wextra")
set(CMAKE_CXX_FLAGS_RELEASE "-O3 -DNDEBUG -Wall -Wextra")

# Default to Release build for performance
if(NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE Release)
endif()

# Include directories
include_directories(src)

# Source files
file(GLOB_RECURSE SOURCES "src/*.cpp" "src/*.hpp" "src/*.h")

# Create executable
add_executable(scuffedredis-server 
    src/server/main.cpp
    src/network/tcp_server.cpp
    src/protocol/request_handler.cpp
    src/data/hashtable.cpp
    src/data/binary_tree.cpp
    src/server/event_loop.cpp
    src/utils/logger.cpp
)

add_executable(scuffedredis-client
    src/client/main.cpp
    src/network/tcp_client.cpp
    src/protocol/request_builder.cpp
    src/utils/logger.cpp
)

# Link libraries
target_link_libraries(scuffedredis-server pthread)
target_link_libraries(scuffedredis-client pthread)

# Install targets
install(TARGETS scuffedredis-server scuffedredis-client
    RUNTIME DESTINATION bin
)
