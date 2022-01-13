#include "Models.h"

#include <algorithm>
#include <fstream>
#include <set>

namespace LFL::Database {

    auto create_database_connection()
    {
        using namespace sqlite_orm;
        auto storage = make_storage(
            "lfl.sqlite",
            make_table(
                "teams",
                make_column("id", &MTeam::id, primary_key()),
                make_column("name", &MTeam::name),
                make_column("games", &MTeam::games),
                make_column("wins", &MTeam::wins),
                make_column("loses", &MTeam::loses),
                make_column("wins_in_overtime", &MTeam::wins_in_overtime),
                make_column("loses_in_overtime", &MTeam::loses_in_overtime),
                make_column("points", &MTeam::points),
                make_column("goals_for", &MTeam::goals_for),
                make_column("goals_again", &MTeam::goals_again),
                make_column("sum_of_attendance", &MTeam::sum_of_attendance)),
            make_table(
                "players",
                make_column("id", &MPlayer::id, primary_key()),
                make_column("number", &MPlayer::number),
                make_column("name", &MPlayer::name),
                make_column("surname", &MPlayer::surname),
                make_column("team_id", &MPlayer::team_id),
                make_column("p_type", &MPlayer::p_type),
                make_column("games", &MPlayer::games),
                make_column("seconds_on_field", &MPlayer::seconds_on_field),
                make_column("yellow_cards", &MPlayer::yellow_cards),
                make_column("red_cards", &MPlayer::red_cards),
                make_column("goals", &MPlayer::goals),
                make_column("assists", &MPlayer::assists),
                make_column("goal_from_penalty", &MPlayer::goal_from_penalty),
                make_column("goalkeper_got_scores",
                            &MPlayer::goalkeper_got_scores)),
            make_table("history",
                       make_column("id", &MMatchHistory::id, primary_key()),
                       make_column("team_id", &MMatchHistory::team_id),
                       make_column("date", &MMatchHistory::date)));
        storage.sync_schema();
        return storage;
    }

    void process_game_info(const LFL::XMLParser::Data::Game &game)
    {
        using namespace sqlite_orm;

        auto storage = create_database_connection();

        auto get_or_create_team = [&storage](const std::string &name) {
            auto team = storage.get_all<MTeam>(where(c(&MTeam::name) == name));

            if (team.size() == 0) {
                // create
                MTeam new_team{
                    -1,
                    name,
                    0,  // games
                    0,  // wins
                    0,  // loses
                    0,  // wins_ot
                    0,  // loses_ot
                    0,  // points
                    0,  // goal_for
                    0,  // goal_again
                    0,  // sun_of_att
                };

                //  insert returns inserted id
                new_team.id = storage.insert(new_team);
                return new_team;
            }
            else {
                assert(team.size() == 1);
                return team[0];
            }
        };

        auto is_match_in_history = [&storage](int team_id,
                                              const std::string &date) {
            auto mh = storage.get_all<MMatchHistory>(
                where(c(&MMatchHistory::team_id) == team_id and
                      c(&MMatchHistory::date) == date));
            return mh.size() == 1;
        };

        auto get_or_create_team_players =
            [&storage](int team_id,
                       const std::vector<XMLParser::Data::Player> &players) {
                std::map<int, MPlayer> res;

                auto team_p = storage.get_all<MPlayer>(
                    where(c(&MPlayer::team_id) == team_id));

                for (auto p : team_p) {
                    res[p.number] = p;
                }

                storage.transaction([&] {
                    for (const auto &xml_player : players) {
                        if (res.count(xml_player.number) == 0) {
                            // create new player
                            MPlayer new_player{
                                -1,
                                xml_player.number,
                                xml_player.name,
                                xml_player.surname,
                                team_id,
                                xml_player.p_type,
                                0,  // games
                                0,  // seconds on the field
                                0,  // yellow cards
                                0,  // read cards
                                0,  // goals
                                0,  // assists
                                0,  // goals from penalty
                                0,  // goals got as goalkeeper
                            };

                            new_player.id = storage.insert(new_player);

                            res[new_player.number] = new_player;
                        }
                    }

                    return true;  // commit
                });

                return res;
            };

        assert(game.teams.size() == 2);
        const auto &team1_data = game.teams[0];
        const auto &team2_data = game.teams[1];

        MTeam team1 = get_or_create_team(team1_data.name);
        MTeam team2 = get_or_create_team(team2_data.name);

        std::cout << team1.name << " vs " << team2.name << " (" << game.date
                  << " @ " << game.place << ")" << std::endl;

        if (is_match_in_history(team1.id, game.date)) {
            std::cout << "\tAlready processed this match! Ignoring\n";
            return;
        }
        else {  // Add to history
            std::cout << "\tProcessing!" << std::endl;

            MMatchHistory mh1{-1, team1.id, game.date};
            storage.insert(mh1);

            MMatchHistory mh2{-1, team2.id, game.date};
            storage.insert(mh2);
        }

        // update games
        team1.games += 1;
        team2.games += 1;

        // update attendance
        team1.sum_of_attendance += game.attendance;
        team2.sum_of_attendance += game.attendance;

        // Update goal_for goal_against
        team1.goals_for += team1_data.goals.size();
        team2.goals_again += team1_data.goals.size();
        team2.goals_for += team2_data.goals.size();
        team1.goals_again += team2_data.goals.size();

        // Finished in main time, or had overtimes?
        const int MAIN_TIME = 60 * 60;  // 60m

        int last_goal = 0;
        for (const auto &x : team1_data.goals) {
            last_goal = std::max(last_goal, x.time);
        }
        for (const auto &x : team2_data.goals) {
            last_goal = std::max(last_goal, x.time);
        }
        bool overtime = last_goal > MAIN_TIME;

        // Overtime end at the exact moment someone score goal
        int match_lenght = std::max(MAIN_TIME, last_goal);

        if (team1_data.goals.size() > team2_data.goals.size()) {
            // team1 won
            if (overtime) {
                team1.wins_in_overtime += 1;
                team1.points += 3;
                team2.loses_in_overtime += 1;
                team2.points += 2;
            }
            else {
                team1.wins += 1;
                team1.points += 5;
                team2.loses += 1;
                team2.points += 1;
            }
        }
        else {
            // team2 won
            if (overtime) {
                team2.wins_in_overtime += 1;
                team2.points += 3;
                team1.loses_in_overtime += 1;
                team1.points += 2;
            }
            else {
                team2.wins += 1;
                team2.points += 5;
                team1.loses += 1;
                team1.points += 1;
            }
        }

        storage.update(team1);
        storage.update(team2);

        std::cout << "\tTeam data updated!" << std::endl;

        auto process_players = [&storage,
                                match_lenght,
                                &get_or_create_team_players](
                                   const auto &our_team,
                                   const auto &en_team,
                                   int team_id) {
            // [numb -> MPlayer]
            auto players =
                get_or_create_team_players(team_id, our_team.players);

            {  // yellow and red cards
                std::set<int> yellow_cards;
                for (const auto &pen : our_team.penalties) {
                    if (yellow_cards.count(pen.number) != 0) {
                        players[pen.number].red_cards += 1;
                        players[pen.number].yellow_cards -= 1;
                    }
                    else {
                        players[pen.number].yellow_cards += 1;
                        yellow_cards.insert(pen.number);
                    }
                }
            }
            {  // goals and assists
                for (const auto &goal : our_team.goals) {
                    if (goal.from_game) {
                        players[goal.number].goals += 1;
                    }
                    else {
                        players[goal.number].goal_from_penalty += 1;
                    }

                    for (int a : goal.assists) {
                        players[a].assists += 1;
                    }
                }
            }

            {  // games player and minutes on the field (and
               // goalkeper_got_scores)
                for (const auto &p : players) {
                    const int my_numb = p.first;

                    int play_time = 0;

                    int last_event_time = 0;  // when last even happened
                    bool is_playing = false;

                    if (std::find(our_team.starting_players.begin(),
                                  our_team.starting_players.end(),
                                  my_numb) != our_team.starting_players.end()) {
                        is_playing = true;
                    }

                    // All goals the goalkeeper got, when they were at field.
                    // not left half-open interval (L; R]
                    auto goalkeeper_got_goals = [&en_team, &players, my_numb](
                                                    int L, int R) {
                        for (const auto &goal : en_team.goals) {
                            if (L < goal.time and goal.time <= R) {
                                players[my_numb].goalkeper_got_scores += 1;
                            }
                        }
                    };

                    for (const auto &sub_event : our_team.subsitutions) {
                        if (sub_event.p_out == my_numb) {
                            assert(is_playing);
                            play_time += (sub_event.time - last_event_time);

                            // if we are goalkeeper last count how many goals we
                            // got in this time frame
                            if (p.second.p_type ==
                                XMLParser::Data::PlayerType::GOALKEEPER) {
                                goalkeeper_got_goals(last_event_time,
                                                     sub_event.time);
                            }

                            is_playing = false;
                            last_event_time = sub_event.time;
                        }
                        else if (sub_event.p_in == my_numb) {
                            assert(is_playing == false);
                            is_playing = true;

                            last_event_time = sub_event.time;
                        }
                    }

                    if (is_playing) {
                        // player played till the end
                        play_time += (match_lenght - last_event_time);

                        // if we are goalkeeper last count how many goals we got
                        // in this time frame
                        if (p.second.p_type ==
                            XMLParser::Data::PlayerType::GOALKEEPER) {
                            goalkeeper_got_goals(last_event_time, match_lenght);
                        }
                    }

                    // Player played the game, iff they were at least one second
                    // in the game
                    if (play_time > 0) {
                        players[my_numb].games += 1;
                        players[my_numb].seconds_on_field += play_time;
                    }
                }
            }

            // commit all changes
            storage.transaction([&] {
                for (auto x : players) {
                    storage.update(x.second);
                }

                return true;
            });
        };

        process_players(team1_data, team2_data, team1.id);
        std::cout << "\tFirst team players updated!" << std::endl;
        process_players(team2_data, team1_data, team2.id);
        std::cout << "\tSecond team players updated!" << std::endl;
    }

    static std::string generate_header()
    {
        return R"XXX(
<!DOCTYPE html>
<html lang="en">
<head>
  <title>LFL statistics</title>
  <meta charset="utf-8">
  <meta name="viewport" content="width=device-width, initial-scale=1">
  <link rel="stylesheet" href="https://maxcdn.bootstrapcdn.com/bootstrap/3.4.1/css/bootstrap.min.css">
  <script src="https://ajax.googleapis.com/ajax/libs/jquery/3.5.1/jquery.min.js"></script>
  <script src="https://maxcdn.bootstrapcdn.com/bootstrap/3.4.1/js/bootstrap.min.js"></script>
</head>
<body>

<nav class="navbar navbar-expand-lg navbar-light bg-light">
    <a class="btn btn-primary" href="#club">Club table</a>
    <a class="btn btn-primary" href="#best_striker">Best stiker</a>
    <a class="btn btn-primary" href="#mvp">MVP</a>
    <a class="btn btn-primary" href="#goalkeeper">Best goalkeeper</a>
    <a class="btn btn-primary" href="#hard_work">Most hardworking</a>
    <a class="btn btn-primary" href="#popular">Most popular club</a>
</nav>
)XXX";
    }

    static std::string generate_footer()
    {
        return R"XXX(
</body>
</html>
)XXX";
    }

    static std::string main_table_header()
    {
        return R"""(
<table class="table table-hover table-responsive table-sm" id="club">
  <thead>
    <tr>
      <th scope="col">Position</th>
      <th scope="col">Club</th>
      <th scope="col">Played</th>
      <th scope="col">Won</th>
      <th scope="col">Lost</th>
      <th scope="col">Won (OT)</th>
      <th scope="col">Lost (OT)</th>
      <th scope="col">GF</th>
      <th scope="col">GA</th>
      <th scope="col">GD</th>
      <th scope="col">Points</th>
    </tr>
  </thead>
  <tbody>
)""";
    }

    static std::string best_striker_header()
    {
        return R"""(
<table class="table table-hover table-responsive table-sm" id="best_striker">
  <thead>
    <tr>
      <th scope="col">#</th>
      <th scope="col">Player</th>
      <th scope="col">Club</th>
      <th scope="col">Points</th>
      <th scope="col">Goals(penalty)</th>
      <th scope="col">Assists</th>
      <th scope="col">Games played</th>
    </tr>
  </thead>
  <tbody>
)""";
    }

    static std::string best_scoring_header()
    {
        return R"""(
<table class="table table-hover table-responsive table-sm" id="mvp">
  <thead>
    <tr>
      <th scope="col">#</th>
      <th scope="col">Player</th>
      <th scope="col">Club</th>
      <th scope="col">Points/h</th>
      <th scope="col">Points</th>
      <th scope="col">Games played</th>
      <th scope="col">Minutes on the field</th>
    </tr>
  </thead>
  <tbody>
)""";
    }

    static std::string best_goalkeeper_header()
    {
        return R"""(
<table class="table table-hover table-responsive table-sm" id="goalkeeper">
  <thead>
    <tr>
      <th scope="col">#</th>
      <th scope="col">Player</th>
      <th scope="col">Club</th>
      <th scope="col">Goal agains/h</th>
      <th scope="col">Goal agains</th>
      <th scope="col">Games played</th>
      <th scope="col">Minutes on the field</th>
    </tr>
  </thead>
  <tbody>
)""";
    }

    static std::string best_hardworking_header()
    {
        return R"""(
<table class="table table-hover table-responsive table-sm" id="hard_work">
  <thead>
    <tr>
      <th scope="col">#</th>
      <th scope="col">Player</th>
      <th scope="col">Club</th>
      <th scope="col">Minutes on the field</th>
    </tr>
  </thead>
  <tbody>
)""";
    }

    static std::string most_popular_club_header()
    {
        return R"""(
<table class="table table-hover table-responsive table-sm" id="popular">
  <thead>
    <tr>
      <th scope="col">Position</th>
      <th scope="col">Club</th>
      <th scope="col">Avg attendances</th>
    </tr>
  </thead>
  <tbody>
)""";
    }

    void generate_html_output(const std::string &filename,
                              size_t truncate_after)
    {
        double start_time = clock();

        auto storage = create_database_connection();

        auto teams = storage.get_all<MTeam>();
        auto players = storage.get_all<MPlayer>();

        std::cout << "Processing " << teams.size() << " teams!" << std::endl;
        std::cout << "And " << players.size() << " players!" << std::endl;

        if (truncate_after == 0u or truncate_after >= players.size()) {
            truncate_after = static_cast<int>(players.size());
            std::cout << "Creating full tables with " << truncate_after
                      << " rows!" << std::endl;
        }
        else {
            std::cout << "Truncating tables after " << truncate_after
                      << " rows!" << std::endl;
        }

        std::ofstream ofs(filename);

        ofs << generate_header();

        std::map<int, std::string> team_names;
        for (const auto c : teams) {
            team_names[c.id] = c.name;
        }

        {  // club table
            ofs << main_table_header();

            // Sort by (score, gd)
            std::sort(teams.begin(), teams.end(), [](MTeam a, MTeam b) {
                if (a.points != b.points)
                    return a.points > b.points;
                if (a.gd() != b.gd())
                    return a.gd() > b.gd();
                return false;
            });

            for (size_t i = 0; i < teams.size(); i++) {
                const auto &t = teams[i];
                ofs << "\t<tr>\n";
                ofs << "\t\t<th scope=\"row\">" << (i + 1) << "</th>\n";
                ofs << "\t\t<td>" << t.name << "</td>\n";
                ofs << "\t\t<td>" << t.games << "</td>\n";
                ofs << "\t\t<td>" << t.wins << "</td>\n";
                ofs << "\t\t<td>" << t.loses << "</td>\n";
                ofs << "\t\t<td>" << t.wins_in_overtime << "</td>\n";
                ofs << "\t\t<td>" << t.loses_in_overtime << "</td>\n";
                ofs << "\t\t<td>" << t.goals_for << "</td>\n";
                ofs << "\t\t<td>" << t.goals_again << "</td>\n";
                ofs << "\t\t<td>" << t.gd() << "</td>\n";
                ofs << "\t\t<td>" << t.points << "</td>\n";
                ofs << "\t</tr>\n";
            }
            ofs << "</tbody>\n";
            ofs << "</table>\n";
        }

        {  // best striker (points)
            ofs << best_striker_header();
            // Sort by (points, goals, p/game)
            std::sort(players.begin(), players.end(), [](MPlayer a, MPlayer b) {
                if (a.points() != b.points())
                    return a.points() > b.points();
                if (a.sum_goals() != b.sum_goals())
                    return a.sum_goals() > b.sum_goals();
                if (a.point_per_h() != b.point_per_h())
                    return a.point_per_h() > b.point_per_h();
                return false;
            });

            for (size_t i = 0; i < truncate_after; i++) {
                const auto &p = players[i];
                ofs << "\t<tr>\n";
                ofs << "\t\t<th scope=\"row\">" << (i + 1) << "</th>\n";
                ofs << "\t\t<td>" << p.name << " " << p.surname << " ("
                    << p.number << ")</td>\n";
                ofs << "\t\t<td>" << team_names.at(p.team_id) << "</td>\n";
                ofs << "\t\t<td>" << p.points() << "</td>\n";
                ofs << "\t\t<td>" << p.sum_goals() << "(" << p.goal_from_penalty
                    << ")</td>\n";
                ofs << "\t\t<td>" << p.assists << "</td>\n";
                ofs << "\t\t<td>" << p.games << "</td>\n";
                ofs << "\t</tr>\n";
            }

            ofs << "</tbody>\n";
            ofs << "</table>\n";
        }
        {  // MVP (point/h)
            ofs << best_scoring_header();
            // Sort by (points, goals, p/game)
            std::sort(players.begin(), players.end(), [](MPlayer a, MPlayer b) {
                if (a.point_per_h() != b.point_per_h())
                    return a.point_per_h() > b.point_per_h();
                if (a.seconds_on_field != b.seconds_on_field)
                    return a.seconds_on_field > b.seconds_on_field;
                return false;
            });

            for (size_t i = 0; i < truncate_after; i++) {
                const auto &p = players[i];
                ofs << "\t<tr>\n";
                ofs << "\t\t<th scope=\"row\">" << (i + 1) << "</th>\n";
                ofs << "\t\t<td>" << p.name << " " << p.surname << " ("
                    << p.number << ")</td>\n";
                ofs << "\t\t<td>" << team_names.at(p.team_id) << "</td>\n";
                ofs << "\t\t<td>" << p.point_per_h() << "</td>\n";
                ofs << "\t\t<td>" << p.points() << "</td>\n";
                ofs << "\t\t<td>" << p.games << "</td>\n";
                ofs << "\t\t<td>" << p.minutes_on_field() << "</td>\n";
                ofs << "\t</tr>\n";
            }

            ofs << "</tbody>\n";
            ofs << "</table>\n";
        }

        {  // best goalie
            ofs << best_goalkeeper_header();
            std::vector<MPlayer> goalies;
            for (const auto &p : players) {
                if (p.p_type == XMLParser::Data::PlayerType::GOALKEEPER) {
                    goalies.push_back(p);
                }
            }

            std::sort(goalies.begin(), goalies.end(), [](MPlayer a, MPlayer b) {
                if (a.gaa_per_h() != b.gaa_per_h())
                    return a.gaa_per_h() < b.gaa_per_h();
                if (a.seconds_on_field != b.seconds_on_field)
                    return a.seconds_on_field > b.seconds_on_field;
                return false;
            });

            for (size_t i = 0; i < std::min(truncate_after, goalies.size());
                 i++) {
                const auto &p = goalies[i];
                ofs << "\t<tr>\n";
                ofs << "\t\t<th scope=\"row\">" << (i + 1) << "</th>\n";
                ofs << "\t\t<td>" << p.name << " " << p.surname << " ("
                    << p.number << ")</td>\n";
                ofs << "\t\t<td>" << team_names.at(p.team_id) << "</td>\n";
                ofs << "\t\t<td>" << p.gaa_per_h() << "</td>\n";
                ofs << "\t\t<td>" << p.goalkeper_got_scores << "</td>\n";
                ofs << "\t\t<td>" << p.games << "</td>\n";
                ofs << "\t\t<td>" << p.minutes_on_field() << "</td>\n";
                ofs << "\t</tr>\n";
            }
            ofs << "</tbody>\n";
            ofs << "</table>\n";
        }

        {  // Hard working (seconds player)
            ofs << best_hardworking_header();
            // Sort by (seconds)
            std::sort(players.begin(), players.end(), [](MPlayer a, MPlayer b) {
                if (a.seconds_on_field != b.seconds_on_field)
                    return a.seconds_on_field > b.seconds_on_field;
                return false;
            });

            for (size_t i = 0; i < truncate_after; i++) {
                const auto &p = players[i];
                ofs << "\t<tr>\n";
                ofs << "\t\t<th scope=\"row\">" << (i + 1) << "</th>\n";
                ofs << "\t\t<td>" << p.name << " " << p.surname << " ("
                    << p.number << ")</td>\n";
                ofs << "\t\t<td>" << team_names.at(p.team_id) << "</td>\n";
                ofs << "\t\t<td>" << p.minutes_on_field() << "</td>\n";
                ofs << "\t</tr>\n";
            }

            ofs << "</tbody>\n";
            ofs << "</table>\n";
        }

        {  // Most popular club
            ofs << most_popular_club_header();

            // Sort by (avg attendances)
            std::sort(teams.begin(), teams.end(), [](MTeam a, MTeam b) {
                if (a.games == b.games) {
                    if (a.sum_of_attendance != b.sum_of_attendance)
                        return a.points > b.points;
                    return false;
                }
                else {
                    return a.sum_of_attendance * 1LL * b.games >
                           b.sum_of_attendance * 1LL * a.games;
                }
            });

            for (size_t i = 0; i < teams.size(); i++) {
                const auto &t = teams[i];
                ofs << "\t<tr>\n";
                ofs << "\t\t<th scope=\"row\">" << (i + 1) << "</th>\n";
                ofs << "\t\t<td>" << t.name << "</td>\n";
                ofs << "\t\t<td>" << t.avg_attendances() << "</td>\n";
                ofs << "\t</tr>\n";
            }
            ofs << "</tbody>\n";
            ofs << "</table>\n";
        }

        double took_to_generate =
            (clock() - start_time) / CLOCKS_PER_SEC * 1000;
        std::cout << "generated in " << took_to_generate << " ms" << std::endl;
        ofs << "<div>Generation took aprox. " << took_to_generate
            << " ms.</div>\n";
        ofs << "<div>LFL 2022 (c)Aleksandrs Zajakins</div>\n";

        ofs << generate_footer();
        ofs.close();
    }
}  // namespace LFL::Database
