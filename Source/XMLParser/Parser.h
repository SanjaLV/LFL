#pragma once

#include <iostream>
#include <map>
#include <string>
#include <vector>

#include "rapidxml.hpp"

namespace LFL::XMLParser {
    typedef std::map<std::string, std::string> StringsMap;

    namespace Data {
        enum PlayerType { ATTACKER = 0, DEFENDER = 1, GOALKEEPER = 2 };
        /// Base XML parsable object
        class ParsableBaseObject {
        };
        /// Primitive parsable object (in the sense that XML node, does not have
        /// any child notes, so we can construct it just from nodes attributes)
        class PrimitiveParsableObject : public ParsableBaseObject {
        };
        /// Opposite to PrimitiveParsableObject, complex object with sub-nodes,
        /// that requires to traverse XML tree.
        class ParsableObject : public ParsableBaseObject {
        };
        /// Base class for all human classes.
        class Person : public PrimitiveParsableObject {
        public:
            std::string name;
            std::string surname;

            Person(const StringsMap &attr);
        };
        class Referee : public Person {
        public:
            /// Special main referee that is VT XML node
            bool main = false;
            using Person::Person;
            void set_main(bool is_main) { main = is_main; }
        };
        class Player : public Person {
        public:
            PlayerType p_type;
            int number;

            Player(const StringsMap &attr);
        };
        /// Base class for almost all timed events
        /// the only exception is non-primitive Goal class.
        class TimedEvent : public PrimitiveParsableObject {
        public:
            /// In seconds from start of the match
            int time;

            TimedEvent(const StringsMap &attr);
        };
        class Substitution : public TimedEvent {
        public:
            int p_out;
            int p_in;

            Substitution(const StringsMap &attr);
        };
        class Penalty : public TimedEvent {
        public:
            int number;

            Penalty(const StringsMap &attr);
        };
        class Goal : public ParsableObject {
        public:
            int time;
            int number;
            bool from_game;
            std::vector<int> assists;

            Goal(rapidxml::xml_node<> *node);
        };
        class Team : public ParsableObject {
        public:
            std::string name;
            std::vector<Player> players;
            std::vector<Substitution> subsitutions;
            std::vector<int> starting_players;
            std::vector<Penalty> penalties;
            std::vector<Goal> goals;

            Team(rapidxml::xml_node<> *node);
        };
        class Game : public ParsableObject {
        public:
            std::string date;
            int attendance;
            std::string place;
            std::vector<Team> teams;
            std::vector<Referee> referees;

            Game(rapidxml::xml_node<> *node);
        };

    }  // namespace Data

    /// Returns XML node attributes as std::string dictionary (map).
    StringsMap parse_node_attributes(rapidxml::xml_node<> *node);
    /// Traverses and parse all XML tree
    /// \return Parsed and ready to consume LFL::Data::Game objects
    Data::Game parse_game_file(const std::string &filename);
}  // namespace LFL::XMLParser
