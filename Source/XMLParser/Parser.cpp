#include <iostream>

#include "Parser.h"

#include "rapidxml_utils.hpp"

namespace LFL::XMLParser {

    using rapidxml::xml_node;
    using rapidxml::xml_attribute;

    Data::Player Parser::parse_player(xml_node<> * node) {
        assert(node != nullptr);

        Data::Player res;

        
        if (xml_attribute<> *attr = node->first_attribute("Loma")) {
            res.p_type = Data::player_type_from_string(attr->value());
        }
        if (xml_attribute<> *attr = node->first_attribute("Uzvards")) {
            res.surname = std::string(attr->value());
        }
        if (xml_attribute<> *attr = node->first_attribute("Vards")) {
            res.name = std::string(attr->value());
        }
        if (xml_attribute<> *attr = node->first_attribute("Nr")) {
            res.number = std::stoi(attr->value());
        }

        return res;

    }

    void Parser::parse_game_file(const std::string& filename) {
        rapidxml::file<> file(filename.c_str());
        rapidxml::xml_document<> doc;
        doc.parse<0>(file.data());

        xml_node<> * root_game_node = doc.first_node("Spele");
        if (root_game_node == nullptr) {
            std::cerr << "Cannot find root XML node Spele!" << std::endl;
            return;
        }
    }

}
