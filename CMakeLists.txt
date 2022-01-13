cmake_minimum_required(VERSION 3.18)

project(LFL VERSION 0.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED True)
set(CMAKE_CXX_EXTENSIONS OFF)

if (MSVC)
    message(WARNING "Windows is not tested")
    set(CMAKE_CXX_FLAGS "/O2 /Wall /Wextra /Wshadow /Wpedantic /Werror")
else()
    set(CMAKE_CXX_FLAGS "-O2 -Wall -Wextra -Wshadow -Wpedantic -Werror -fmax-errors=3")
endif()

find_package (SQLite3 REQUIRED)



add_subdirectory(ThirdParty)
add_subdirectory(Source)

