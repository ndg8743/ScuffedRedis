# Test configuration for ScuffedRedis
cmake_minimum_required(VERSION 3.10)

# Enable testing
enable_testing()

# Basic functionality tests
add_executable(test_basic test_basic.cpp 
    ../src/data/hashtable.cpp
    ../src/protocol/protocol.cpp
    ../src/data/ttl_manager.cpp
)

# Link with required libraries
if(WIN32)
    target_link_libraries(test_basic ws2_32)
endif()

# Add test
add_test(NAME basic_tests COMMAND test_basic)
