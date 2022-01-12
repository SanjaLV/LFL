#include <iostream>
#include <functional>

#include "Parser.h"

#include "rapidxml_utils.hpp"

namespace LFL::XMLParser {
    using rapidxml::xml_node;
    using rapidxml::xml_attribute;

    StringsMap parse_node_attributes(xml_node<> *node) {
        assert(node != nullptr);

        StringsMap res;
        for (auto attr = node->first_attribute(); attr; attr = attr->next_attribute()) {
            res[attr->name()] = attr->value();
        }

        return res;
    }

    template <typename T>
    static T parse_primitive_data_object(xml_node<> *node) {
        const auto values = parse_node_attributes(node);
        return T(values);
    }
    template <typename T>
    static std::vector<T> parse_multiple_primitives(xml_node<> *node, const char * node_text) {
        std::vector<T> res;
        for (auto * son = node->first_node(node_text); son; son = son->next_sibling(node_text)) {
            res.emplace_back(parse_primitive_data_object<T>(son));
        }
        return res;
    }

    template<typename T>
    static std::vector<T> parse_multiple_non_primitives(xml_node<> *node, const char * node_text) {
        std::vector<T> res;
        for (auto * son = node->first_node(node_text); son; son = son->next_sibling(node_text)) {
            res.emplace_back(T(son));
        }
        return res;
    }

    Data::Game parse_game_file(const std::string& filename) {
        rapidxml::file<> file(filename.c_str());
        rapidxml::xml_document<> doc;
        doc.parse<0>(file.data());

        xml_node<> * root_game_node = doc.first_node("Spele");
        if (root_game_node == nullptr) {
            std::cerr << "XML is corrupted, there is no root node Spele!" << std::endl;
            assert(false);
        }

        return Data::Game(root_game_node);
    }
}

namespace LFL::XMLParser::Data {

    static int parse_time_from_string(const std::string& str) {
        auto div = str.find(':');

        assert(div != std::string::npos);
        return std::stoi(str.substr(0, div)) * 60 + std::stoi(str.substr(div+1));
    }

    static bool parse_goal_type(const std::string& str) {
        if (str == "J") return false;
        if (str == "N") return true;
        assert(false);
    }

    static PlayerType parse_player_type_from_string(const std::string& str) {
            if (str == "U") {
                return PlayerType::ATTACKER;
            }
            else if (str == "A") {
                return PlayerType::DEFENDER;
            }
            else if (str == "V") {
                return PlayerType::GOALKEEPER;
            }
            else {
                std::cerr << "Unknown PlayerType '" << str << "'";
                assert(false);
            }
    }

    Person::Person(const StringsMap& attr)
        : name(attr.at("Vards"))
        , surname(attr.at("Uzvards"))
    {}

    Player::Player (const StringsMap& attr)
        : Person(attr)
        , p_type(parse_player_type_from_string(attr.at("Loma")))
        , number(std::stoi(attr.at("Nr")))
    {}

    TimedEvent::TimedEvent(const StringsMap& attr)
        : time(parse_time_from_string(attr.at("Laiks")))
    {}

    Substitution::Substitution(const StringsMap& attr)
        : TimedEvent(attr)
        , p_out(std::stoi(attr.at("Nr1")))
        , p_in(std::stoi(attr.at("Nr2")))
    {}

    Penalty::Penalty (const StringsMap& attr)
        : TimedEvent(attr)
        , number(std::stoi(attr.at("Nr")))
    {}

    Goal::Goal (rapidxml::xml_node<char> * node)
    {
        const auto attr = parse_node_attributes(node);

        time = parse_time_from_string(attr.at("Laiks"));
        number = std::stoi(attr.at("Nr"));
        from_game = parse_goal_type(attr.at("Sitiens"));

        for (auto * son = node->first_node("P"); son; son = son->next_sibling("P")) {
            assists.emplace_back(std::stoi(XMLParser::parse_node_attributes(son).at("Nr")));
        }
    }

    Team::Team(rapidxml::xml_node<> * node)
    {
        const auto attr = parse_node_attributes(node);

        name = attr.at("Nosaukums");

        if (auto subnode = node->first_node("Speletaji")) {
            players = parse_multiple_primitives<Player>(subnode, "Speletajs");
        }
        else {
            std::cerr << "XML is corrupted, there is no Speletaji subnode!";
            assert(false);
        }
        if (auto subnode = node->first_node("Mainas")) {
            subsitutions = parse_multiple_primitives<Substitution>(subnode, "Maina");
        }
        if (auto subnode = node->first_node("Pamatsastavs")) {
            for (auto * son = subnode->first_node("Speletajs"); son; son = son->next_sibling("Speletajs")) {
                starting_players.push_back(std::stoi(parse_node_attributes(son).at("Nr")));
            }
        }
        else {
            std::cerr << "XML is corrupted, there is no Pamatsastavs subnode!";
            assert(false);
        }
        if (auto subnode = node->first_node("Sodi")) {
            penalties = parse_multiple_primitives<Penalty>(subnode, "Sods");
        }
        if (auto subnode = node->first_node("Varti")) {
            goals = parse_multiple_non_primitives<Goal>(subnode, "VG");
        }
    }

    Game::Game(rapidxml::xml_node<> * node)
    {
        const auto attr = parse_node_attributes(node);

        date = attr.at("Laiks");
        place = attr.at("Vieta");
        attendance = std::stoi("6740");

        teams = parse_multiple_non_primitives<Team>(node, "Komanda");
        referees = parse_multiple_primitives<Referee>(node, "T");

        if (auto subnode = node->first_node("VT")) {
            Referee main_ref = parse_primitive_data_object<Referee>(subnode);
            main_ref.set_main(true);
            referees.emplace_back(main_ref);
        }
    }
}
