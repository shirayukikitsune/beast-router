cmake_minimum_required(VERSION 3.15)
project(beast_router VERSION 1.0.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED on)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

find_library(Boost 1.71.0 REQUIRED coroutine)

option(EnableTest "Enable unit testing" ON)

add_subdirectory(src)

if (EnableTest)
    enable_testing()
    add_subdirectory(test)
endif()
