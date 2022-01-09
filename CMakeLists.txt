cmake_minimum_required(VERSION 3.18)

project(LFL VERSION 0.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED True)

add_executable(lfl source/lfl/lfl.cpp)
add_executable(lfl-test source/lfl-test/lfl-test.cpp)
