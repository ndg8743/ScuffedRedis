cmake_minimum_required(VERSION 3.10)
project(ScuffedRedis)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Server executable
add_executable(redis-server
    src/server/main.cpp
    src/server/kv_store.cpp
    src/server/command_handler.cpp
    src/network/socket.cpp
    src/network/tcp_server.cpp
    src/protocol/protocol.cpp
    src/data/hashtable.cpp
    src/data/sorted_set.cpp
    src/data/ttl_manager.cpp
    src/event/event_loop.cpp
)

# Client executable (optional)
add_executable(redis-client
    src/client/main.cpp
    src/client/redis_client.cpp
    src/network/socket.cpp
    src/network/tcp_client.cpp
    src/protocol/protocol.cpp
)

# Include directories
target_include_directories(redis-server PRIVATE src)
target_include_directories(redis-client PRIVATE src)

# Platform-specific settings
if(WIN32)
    target_link_libraries(redis-server ws2_32)
    target_link_libraries(redis-client ws2_32)
else()
    find_package(Threads REQUIRED)
    target_link_libraries(redis-server Threads::Threads)
    target_link_libraries(redis-client Threads::Threads)
endif()

# Enable warnings
if(MSVC)
    target_compile_options(redis-server PRIVATE /W4)
    target_compile_options(redis-client PRIVATE /W4)
else()
    target_compile_options(redis-server PRIVATE -Wall -Wextra -Wpedantic)
    target_compile_options(redis-client PRIVATE -Wall -Wextra -Wpedantic)
endif()

# Debug/Release settings
set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} -g")
set(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} -O3")