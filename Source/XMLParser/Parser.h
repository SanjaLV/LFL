#pragma once

#include <string>
#include <iostream>
#include <vector>
#include <map>


#include "rapidxml.hpp"

namespace LFL::XMLParser {
    typedef std::map<std::string,std::string> StringsMap;

    namespace Data {
        enum PlayerType {
            ATTACKER = 0,
            DEFENDER = 1,
            GOALKEEPER = 2
        };
        class ParsableBaseObject {
        };
        class PrimitiveParsableObject : public ParsableBaseObject {
        };
        class ParsableObject : public ParsableBaseObject {
        };
        class Person : public PrimitiveParsableObject {
            public:
                std::string name;
                std::string surname;

                Person (const StringsMap& attr);
        };
        class Referee : public Person {
            public:
                bool main = false;
                using Person::Person;
                void set_main(bool is_main) {main = is_main;}
        };
        class Player : public Person {
            public:
                PlayerType p_type;
                int number;

                Player(const StringsMap& attr);
        };
        class TimedEvent : public PrimitiveParsableObject {
            public:
                int time;

                TimedEvent(const StringsMap& attr);
        };
        class Substitution : public TimedEvent {
            public:
                int p_out;
                int p_in;

                Substitution(const StringsMap& attr);
        };
        class Penalty : public TimedEvent {
            public:
                int number;

                Penalty (const StringsMap& attr);
        };
        class Goal : public ParsableObject {
            public:
                int time;
                int number;
                bool from_game;
                std::vector<int> assists;

                Goal (rapidxml::xml_node<> * node);
        };
        class Team : public ParsableObject {
            public:
                std::string name;
                std::vector<Player> players;
                std::vector<Substitution> subsitutions;
                std::vector<int> starting_players;
                std::vector<Penalty> penalties;
                std::vector<Goal> goals;

                Team(rapidxml::xml_node<> * node);
        };
        class Game : public ParsableObject {
            private:
                std::string date;
                int attendance;
                std::string place;
                std::vector<Team> teams;
                std::vector<Referee> referees;
            public:
                Game(rapidxml::xml_node<> * node);
        };

    }

    StringsMap parse_node_attributes(rapidxml::xml_node<> * node);
    Data::Game parse_game_file(const std::string& filename);
}
