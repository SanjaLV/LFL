set(sources
    XMLParserTest.cpp
    XMLParserTest.h
    lfl-test.cpp
)

source_group("Source" FILES ${sources})

add_executable(lfl-test ${sources})

target_link_libraries(lfl-test XMLParser)
target_compile_options(lfl-test PRIVATE "-DLFL_TEST=1 -g -fsanitize=undefined,address")

add_custom_command(TARGET lfl-test 
                   POST_BUILD
                   COMMAND ${CMAKE_COMMAND} -E copy $<TARGET_FILE:lfl-test> ${CMAKE_BINARY_DIR})
