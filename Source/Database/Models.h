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
        int team_id;  // Link to TeamDB
        int p_type;

        int games;
        int seconds_on_field;
        int yellow_cards;
        int red_cards;

        int goals;
        int assists;
        int goal_from_penalty;

        int goalkeper_got_scores;  // to calc avg/game
                                   // and avg/minutes_on_field

        int points() const { return goals + assists + goal_from_penalty; }
        int sum_goals() const { return goals + goal_from_penalty; }
        double point_per_h() const
        {
            if (seconds_on_field == 0)
                return 0;
            else
                return static_cast<double>(points()) / seconds_on_field * 60 *
                       60;
        }
        double minutes_on_field() const { return seconds_on_field / 60.0; }

        double gaa_per_h() const
        {
            if (seconds_on_field == 0)
                return 0;
            else
                return static_cast<double>(goalkeper_got_scores) /
                       seconds_on_field * 60 * 60;
        }
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

        int sum_of_attendance;  // to calculate avg attendance

        int gd() const { return goals_for - goals_again; }

        double avg_attendances() const
        {
            if (games == 0)
                return 0;
            return static_cast<double>(sum_of_attendance) / games;
        }
    };

    struct MMatchHistory {
        int id = 0;

        int team_id;  // Link to TeamDB
        std::string date;
    };

    auto create_database_connection();
    void process_game_info(const LFL::XMLParser::Data::Game &game);
    void generate_html_output(const std::string &filename);

}  // namespace LFL::Database
