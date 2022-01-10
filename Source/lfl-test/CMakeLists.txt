set(sources
    XMLParserTest.cpp
    XMLParserTest.h
    lfl-test.cpp
)

source_group("Source" FILES ${sources})

add_executable(lfl-test ${sources})

target_link_libraries(lfl-test XMLParser)

add_custom_command(TARGET lfl-test 
                   POST_BUILD
                   COMMAND ${CMAKE_COMMAND} -E copy $<TARGET_FILE:lfl-test> ${CMAKE_BINARY_DIR})
