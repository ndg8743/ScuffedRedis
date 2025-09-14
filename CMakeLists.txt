cmake_minimum_required(VERSION 3.10)
project(ScuffedRedis VERSION 1.0.0 LANGUAGES CXX)

# Set C++ standard
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Compiler flags for optimization and warnings
if(CMAKE_CXX_COMPILER_ID MATCHES "GNU|Clang")
    add_compile_options(-Wall -Wextra -Wpedantic -O2)
    # Debug flags
    if(CMAKE_BUILD_TYPE STREQUAL "Debug")
        add_compile_options(-g -O0 -fsanitize=address)
        add_link_options(-fsanitize=address)
    endif()
elseif(MSVC)
    add_compile_options(/W4 /O2)
    if(CMAKE_BUILD_TYPE STREQUAL "Debug")
        add_compile_options(/Od /Zi)
    endif()
endif()

# Include directories
include_directories(${CMAKE_SOURCE_DIR}/src)

# Source files - will be updated as we add components
set(SERVER_SOURCES
    src/server/main.cpp
)

set(CLIENT_SOURCES
    src/client/main.cpp
)

# Build server executable
add_executable(scuffed-redis-server ${SERVER_SOURCES})

# Build client executable  
add_executable(scuffed-redis-cli ${CLIENT_SOURCES})

# Platform-specific network libraries
if(WIN32)
    target_link_libraries(scuffed-redis-server ws2_32)
    target_link_libraries(scuffed-redis-cli ws2_32)
endif()

# Enable testing
enable_testing()
add_subdirectory(tests EXCLUDE_FROM_ALL)
