#pragma once

#include <string>

#include <sqlite_orm.h>

#include "XMLParser/Parser.h"

namespace LFL::Database {

    /// Database model class that represents Player entity
    struct MPlayer {
        int id = 0;
        int number;
        std::string name;
        std::string surname;
        /// Link to MTeam table
        int team_id;
        int p_type;

        int games;
        int seconds_on_field;
        int yellow_cards;
        int red_cards;

        int goals;
        int assists;
        int goal_from_penalty;

        /// How many goal was score against goalie
        /// it is needed to compute gaa_per_h().
        int goalkeper_got_scores;  // to calc avg/game
                                   // and avg/minutes_on_field

        int points() const { return goals + assists + goal_from_penalty; }
        int sum_goals() const { return goals + goal_from_penalty; }

        /// Average points per every hour played(higher is better)
        double point_per_h() const
        {
            if (seconds_on_field == 0)
                return 0;
            else
                return static_cast<double>(points()) / seconds_on_field * 60 *
                       60;
        }
        double minutes_on_field() const { return seconds_on_field / 60.0; }

        /// Goal against goal for every hour played (lower is better).
        double gaa_per_h() const
        {
            if (seconds_on_field == 0)
                return 0;
            else
                return static_cast<double>(goalkeper_got_scores) /
                       seconds_on_field * 60 * 60;
        }
    };

    /// Database model class that represents team entity
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

        /// Calculates goal deficit
        /// \returns goal_for - goal_against
        int gd() const { return goals_for - goals_again; }

        /// Calculates average per game attendances
        double avg_attendances() const
        {
            if (games == 0)
                return 0;
            return static_cast<double>(sum_of_attendance) / games;
        }
    };

    /// Database model, that contains match history to detect duplicates
    struct MMatchHistory {
        int id = 0;
        /// Link to MTeam table
        int team_id;
        std::string date;
    };

    /// Creates and loads database scheme, and creates connections
    /// \returns sqlite_orm database connection
    auto create_database_connection();

    /// Processes and records this game info
    /// \param Parse from XML file game data.
    /// \see LFL::XMLParser::Data::Game
    void process_game_info(const LFL::XMLParser::Data::Game &game);
    /// Generates html report
    /// \param filename path to the generated html file
    /// \param truncate_after maximal row limit in players tables, 0 if
    /// infinite.
    void generate_html_output(const std::string &filename,
                              size_t truncate_after);

}  // namespace LFL::Database
