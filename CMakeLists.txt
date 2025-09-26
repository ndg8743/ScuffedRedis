cmake_minimum_required(VERSION 3.10)
project(RedisServer)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

add_executable(redis-server simple_server.cpp)