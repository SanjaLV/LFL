add_library(rapidxml INTERFACE)
target_include_directories(rapidxml SYSTEM INTERFACE "${CMAKE_CURRENT_SOURCE_DIR}")
add_library(ThirdParty::RapidXml ALIAS rapidxml)
