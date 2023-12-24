cmake_minimum_required(VERSION 3.22)
project(QCC VERSION 1.0
            DESCRIPTION "A compiler"
            LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_STANDARD_REQUIRED True)
set(EXECUTABLE_OUTPUT_PATH "${CMAKE_BINARY_DIR}")

add_subdirectory(src)
