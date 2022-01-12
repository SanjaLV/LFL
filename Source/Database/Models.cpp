#include "Models.h"

#include <set>
#include <algorithm>

namespace LFL::Database {
    
    auto create_database_connection() {
            using namespace sqlite_orm;
            auto storage = make_storage("lfl.sqlite",
                                    make_table("teams",
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
                                    make_table("players",
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
                                                make_column("goalkeper_got_scores", &MPlayer::goalkeper_got_scores)),
                                    make_table("history",
                                                make_column("id", &MMatchHistory::id, primary_key()),
                                                make_column("team_id", &MMatchHistory::team_id),
                                                make_column("date", &MMatchHistory::date)));
            storage.sync_schema();
            return storage;
        }


    void process_game_info(const LFL::XMLParser::Data::Game& game) {
        using namespace sqlite_orm;

        auto storage = create_database_connection();


        auto get_or_create_team = [&storage](const std::string& name) {
            auto team = storage.get_all<MTeam>(where(c(&MTeam::name) == name));

            if (team.size() == 0) {
                // create
                MTeam new_team{
                    -1,
                    name,
                    0, // games
                    0, // wins
                    0, // loses
                    0, // wins_ot
                    0, // loses_ot
                    0, // points
                    0, // goal_for
                    0, // goal_again
                    0, // sun_of_att
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

        auto is_match_in_history = [&storage](int team_id, const std::string& date) {
            auto mh = storage.get_all<MMatchHistory>(where(c(&MMatchHistory::team_id) == team_id and
                                                           c(&MMatchHistory::date) == date));
            return mh.size() == 1;
        };

        auto get_or_create_team_players = [&storage](int team_id, const std::vector<XMLParser::Data::Player>& players) {
            std::map<int, MPlayer> res;

            auto team_p = storage.get_all<MPlayer>(where(c(&MPlayer::team_id) == team_id));
            
            for (auto p : team_p) {
                res[p.number] = p;
            }

            storage.transaction([&] {
                for (const auto& xml_player : players) {
                    if (res.count(xml_player.number) == 0) {
                        // create new player
                        MPlayer new_player{
                            -1,
                            xml_player.number,
                            xml_player.name,
                            xml_player.surname,
                            team_id,
                            xml_player.p_type,
                            0, // games
                            0, // seconds on the field
                            0, // yellow cards
                            0, // read cards
                            0, // goals
                            0, // assists
                            0, // goals from penalty
                            0, // goals got as goalkeeper
                        };
                        
                        new_player.id = storage.insert(new_player);
                        
                        res[new_player.number] = new_player;
                    }
                }
                
                return true; // commit
            });

            return res;
        };

        assert(game.teams.size() == 2);
        const auto& team1_data = game.teams[0];
        const auto& team2_data = game.teams[1];


        MTeam team1 = get_or_create_team(team1_data.name);
        MTeam team2 = get_or_create_team(team2_data.name);
        
        std::cout << team1.name << " vs " << team2.name << " (" << game.date << " @ " << game.place << ")" << std::endl;

        if (is_match_in_history(team1.id, game.date)) {
            std::cout << "\tAlready processed this match! Ignoring\n";
            return;
        }
        else { // Add to history
            std::cout << "\tProcessing!" << std::endl;
            
            MMatchHistory mh1{
                -1,
                team1.id,
                game.date
            };
            storage.insert(mh1);

            MMatchHistory mh2{
                -1,
                team2.id,
                game.date
            };
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
        const int MAIN_TIME = 60 * 60;       // 60m
        const int OVERTIME_LENGTH = 15 * 60; // 15m

        int last_goal = 0;
        for (const auto& x : team1_data.goals) {
            last_goal = std::max(last_goal, x.time);
        }
        for (const auto& x : team2_data.goals) {
            last_goal = std::max(last_goal, x.time);
        }
        bool overtime = last_goal > MAIN_TIME;

        // Calculate match length it is 60m or in case of OT
        // last_goal rounded up to 15m
        int match_lenght = MAIN_TIME;
        if (overtime) {
            if (last_goal % OVERTIME_LENGTH != 0) {
                match_lenght = last_goal + (OVERTIME_LENGTH - (last_goal % OVERTIME_LENGTH));
            }
            else {
                match_lenght = last_goal;
            }
        }

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
    
        auto process_players = [&storage, match_lenght, &get_or_create_team_players](const auto& our_team, const auto& en_team, int team_id) {
            // [numb -> MPlayer]
            auto players = get_or_create_team_players(team_id, our_team.players);

            { // yellow and red cards
                std::set<int> yellow_cards;
                for (const auto& pen : our_team.penalties) {
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
            { // goals and assists
                for (const auto& goal : our_team.goals) {
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

            { // games player and minutes on the field (and goalkeper_got_scores)
                for (const auto& p : players) {
                    const int my_numb = p.first;

                    int play_time = 0;

                    int last_event_time = 0; // when last even happened
                    bool is_playing = false;

                    if (std::find(our_team.starting_players.begin(),
                                  our_team.starting_players.end(),
                                  my_numb) != our_team.starting_players.end()) {
                        is_playing = true;
                    }

                    // All goals the goalkeeper got, when they were at field.
                    // not left half-open interval (L; R]
                    auto goalkeeper_got_goals = [&en_team, &players, my_numb](int L, int R) {
                        for (const auto& goal : en_team.goals) {
                            if (L < goal.time and goal.time <= R) {
                                players[my_numb].goalkeper_got_scores += 1;
                            }
                        }
                    };

                    for (const auto& sub_event : our_team.subsitutions) {
                        if (sub_event.p_out == my_numb) {
                            assert(is_playing);
                            play_time += (sub_event.time - last_event_time);

                            // if we are goalkeeper last count how many goals we got in this time frame
                            if (p.second.p_type == XMLParser::Data::PlayerType::GOALKEEPER) {
                                goalkeeper_got_goals(last_event_time, sub_event.time);
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

                        // if we are goalkeeper last count how many goals we got in this time frame
                        if (p.second.p_type == XMLParser::Data::PlayerType::GOALKEEPER) {
                            goalkeeper_got_goals(last_event_time, match_lenght);
                        }

                    }

                    // Player played the game, iff they were at least one second in the game
                    if (play_time > 0) {
                        players[my_numb].games += 1;
                        players[my_numb].seconds_on_field += play_time;
                    }
                }
            }

            // commit all changes
            storage.transaction([&]{
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

}
