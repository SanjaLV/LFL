#pragma once

#include <string>
#include <iostream>

#include "rapidxml.hpp"

namespace LFL::Data {

    enum PlayerType {
        ATTACKER = 0,
        DEFENDER = 1,
        GOALKEEPER = 2
    };
    
    __attribute__((weak)) PlayerType player_type_from_string(const std::string& str) {
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

    struct Player {
        PlayerType p_type;
        std::string surname;
        std::string name;
        int number;

        Player(PlayerType _p_type, const std::string& _surname, const std::string& _name, int _number):
            p_type(_p_type), surname(_surname), name(_name), number(_number)
        {}
        Player() {
        }

        const bool operator == (const Player& foo) const {
            if (p_type != foo.p_type) return false;
            // string comparison could be linear, so let's compare number first
            if (number != foo.number) return false;
            if (surname != foo.surname) return false;
            if (name != foo.name) return false;
            return true;
        }
        const bool operator != (const Player& foo) const {
            return !(*this == foo);
        }
    };
}

namespace LFL::XMLParser {

    class Parser {
        public:
            static LFL::Data::Player parse_player(rapidxml::xml_node<> * node);
            static void parse_players(rapidxml::xml_node<> * node);
            static void parse_substitution(rapidxml::xml_node<> * node);
            static void parse_substitutions(rapidxml::xml_node<> * node);
            static void parse_starting(rapidxml::xml_node<> *node);
            static void parse_penaltie(rapidxml::xml_node<> *node);
            static void parse_penalties(rapidxml::xml_node<> * node);
            static void parse_goal(rapidxml::xml_node<> * node);
            static void parse_goals(rapidxml::xml_node<> * node);
            static void parse_judges(rapidxml::xml_node<> * node);
        public:
            static /* LFL::Storages::Game */ void parse_game_file(const std::string& filename);
    };

}
