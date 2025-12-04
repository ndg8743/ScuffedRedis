cmake_minimum_required(VERSION 3.10)
project(ScuffedRedis)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall -Wextra -pedantic")

# Include directories
include_directories(${CMAKE_SOURCE_DIR}/src)

# Simple server executable (for comparison)
add_executable(simple-redis-server src/server/simple_server.cpp)

# Full Redis server implementation
set(SERVER_SOURCES
    src/server/main.cpp
    src/server/command_handler.cpp
    src/server/kv_store.cpp
    src/protocol/protocol.cpp
    src/network/tcp_server.cpp
    src/network/socket.cpp
    src/data/hashtable.cpp
    src/data/sorted_set.cpp
    src/data/ttl_manager.cpp
)

add_executable(scuffed-redis-server ${SERVER_SOURCES})

# Redis client executable
set(CLIENT_SOURCES
    src/client/main.cpp
    src/client/redis_client.cpp
    src/protocol/protocol.cpp
    src/network/tcp_client.cpp
    src/network/socket.cpp
)

add_executable(redis-client ${CLIENT_SOURCES})

# Test executable
if(EXISTS "${CMAKE_SOURCE_DIR}/tests/test_basic.cpp")
    add_executable(test_basic
        tests/test_basic.cpp
        src/data/hashtable.cpp
        src/data/ttl_manager.cpp
        src/protocol/protocol.cpp
    )
endif()

# Platform-specific network libraries
if(WIN32)
    target_link_libraries(scuffed-redis-server ws2_32)
    target_link_libraries(redis-client ws2_32)
    target_link_libraries(simple-redis-server ws2_32)
    if(TARGET test_basic)
        target_link_libraries(test_basic ws2_32)
    endif()
endif()
