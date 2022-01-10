set(sources
    lfl.cpp
)

source_group("Source" FILES ${sources})

add_executable(lfl ${sources})

target_link_libraries(lfl)

add_custom_command(TARGET lfl 
                   POST_BUILD
                   COMMAND ${CMAKE_COMMAND} -E copy $<TARGET_FILE:lfl> ${CMAKE_BINARY_DIR})
