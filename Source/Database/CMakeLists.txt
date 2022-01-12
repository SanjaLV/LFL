set(sources
    Models.h
    Models.cpp
)

source_group("Source" FILES ${sources})
add_library(Database ${sources})
target_include_directories(Database PUBLIC "${CMAKE_CURRENT_BINARY_DIR}")
target_link_libraries(Database XMLParser ThirdParty::SQLite_Orm sqlite3)

    
