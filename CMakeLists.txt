cmake_minimum_required(VERSION 3.18)

project(LFL VERSION 0.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED True)
set(CMAKE_CXX_EXTENSIONS OFF)

add_subdirectory(ThirdParty)
add_subdirectory(Source)

