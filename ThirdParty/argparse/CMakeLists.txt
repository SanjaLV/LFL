add_library(argprase INTERFACE)
target_include_directories(argprase SYSTEM INTERFACE "${CMAKE_CURRENT_SOURCE_DIR}")
add_library(ThirdParty::ArgParse ALIAS argprase)
