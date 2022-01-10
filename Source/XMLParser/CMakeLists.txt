set(sources
    Parser.cpp
    Parser.h
)

source_group("Source" FILES ${sources})
add_library(XMLParser ${sources})
target_include_directories(XMLParser PUBLIC "${CMAKE_CURRENT_BINARY_DIR}")
target_link_libraries(XMLParser ThirdParty::RapidXml)

    
