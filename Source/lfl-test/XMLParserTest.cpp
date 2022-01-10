#include <string>
#include <iostream>

#include "XMLParserTest.h"

#include "rapidxml.hpp"

using rapidxml::xml_document;
using rapidxml::xml_node;

static void TEST_PARSE_PLAYER() {
    char input[] = "<Speletajs Loma='V' Uzvards='Sam' Vards='Sidney' Nr='16'/>";
    xml_document<> doc;
    doc.parse<0>(input);

    xml_node<> * node = doc.first_node("Speletajs");

    if (node == nullptr) {
        std::cerr << "TEST_PARSE_PLAYER: Cannot find Speletajs node";
        return;
    }

    auto res = LFL::XMLParser::Parser::parse_player(node);
    
    LFL::Data::Player should_be(
        LFL::Data::PlayerType::GOALKEEPER,
        "Sam",
        "Sidney",
        16);

    if (res != should_be) {
        std::cerr << "TEST_PARSE_PLAYER: Parsed player does not match expected\n";
        std::cerr << res.p_type << " vs " << should_be.p_type << "\n";
        std::cerr << res.surname << " vs " << should_be.surname << "\n";
        std::cerr << res.name << " vs " << should_be.name << "\n";
        std::cerr << res.number << " vs " << should_be.number << "\n";
    }
}
static void TEST_PARSE_SUBSTITUTION() {
}
static void TEST_PARSE_PENALTY() {
}
static void TEST_PARSE_GOAL() {
}
static void TEST_PARSE_JUDGE() {
}

void TEST_XML_PARSER() {
    TEST_PARSE_PLAYER();
    TEST_PARSE_SUBSTITUTION();
    TEST_PARSE_PENALTY();
    TEST_PARSE_GOAL();
    TEST_PARSE_JUDGE();
}
