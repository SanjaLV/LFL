#pragma once

#include <string>

#include <sqlite_orm.h>

#include "XMLParser/Parser.h"

namespace LFL::Database {

    struct MPlayer {
        int id = 0;
        int number;
        std::string name;
        std::string surname;
        int team_id; // Link to TeamDB
        int p_type;

        int games;
        int seconds_on_field;
        int yellow_cards;
        int red_cards;

        int goals;
        int assists;
        int goal_from_penalty;

        int goalkeper_got_scores; // to calc avg/game
                                  // and avg/minutes_on_field
    };

    struct MTeam {
        int id = 0;
        std::string name;

        int games;
        int wins;
        int loses;
        int wins_in_overtime;
        int loses_in_overtime;
        int points;

        int goals_for;
        int goals_again;

        int sum_of_attendance; // to calculate avg attendance
    };

    struct MMatchHistory {
        int id = 0;

        int team_id; // Link to TeamDB
        std::string date;
    };

    auto create_database_connection(); 
    void process_game_info(const LFL::XMLParser::Data::Game& game);
}

