add_library(sqlite_orm INTERFACE)
target_include_directories(sqlite_orm SYSTEM INTERFACE "${CMAKE_CURRENT_SOURCE_DIR}")
add_library(ThirdParty::SQLite_Orm ALIAS sqlite_orm)
