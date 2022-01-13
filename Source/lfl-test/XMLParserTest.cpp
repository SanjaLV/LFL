#include <iostream>
#include <string>

#include "XMLParserTest.h"

#include "rapidxml.hpp"

using rapidxml::xml_document;
using rapidxml::xml_node;

void TEST_XML_PARSER()
{
    char input[] = "<Speletajs Loma='V' Uzvards='Sam' Vards='Sidney' Nr='16'/>";

    xml_document<> doc;
    doc.parse<0>(input);

    xml_node<> *node = doc.first_node("Speletajs");

    const LFL::XMLParser::StringsMap should_be = {
        {"Loma", "V"},
        {"Uzvards", "Sam"},
        {"Vards", "Sidney"},
        {"Nr", "16"},
    };

    const auto attr = LFL::XMLParser::parse_node_attributes(node);

    if (attr != should_be) {
        std::cerr << "Test: XMLParser::parse_node_attributes: Parsed map does "
                     "not match expected\n";
    }
}
