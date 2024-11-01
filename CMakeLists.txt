cmake_minimum_required(VERSION 3.25)

set(CXX_STANDARD_FLAG "--std=c++20")
set(ENV{CXX} /usr/bin/clang++)
set(CMAKE_CXX_COMPILER /usr/bin/clang++)
set(CMAKE_CXX_FLAGS "${CXX_STANDARD_FLAG} -Wall -Wextra -Werror")

project(VirtualSwitch LANGUAGES CXX)
enable_testing()

# Source code files, excluding the production main file
file(GLOB SRCS src/*.cpp)
list(FILTER SRCS EXCLUDE REGEX "main.cpp")

add_subdirectory(src)
add_subdirectory(tests/unit)
add_subdirectory(tests/e2e)

