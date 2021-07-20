#include <iostream>
#include <string>
#include <vector>
// #include <cstdlib>
#include <chrono>
#include <random>
#include <cmath>

#include "mutils.h" // adds "print" and "str" fuction templates

std::vector<double> prediction_difference;

//
// PLAYER CLASS
//
class Player
{
public:
    // Actual player skill
    double skill;
    // MMR assigned by the matchmaker
    double mmr = 2820 / 2.2;
    // Vector of opponent skills. It's used to define the matchmaker accuracy.
    std::vector<double> opponent_history;
    // MMR history
    std::vector<double> mmr_history;
    // Chances predicted by the matchmaker
    std::vector<double> predicted_chances;
    // Define player and assign him his skill value
    Player(double pskill)
    {
        skill = pskill;
    };
};

//Overload my str fuction for printing the player
std::string str(Player p)
{
    return str("Player(skill=", p.skill) + str(", mmr=", p.mmr) + str(", opponent_history=", p.opponent_history) + str(", mmr_history=", p.mmr_history, ")");
}

//
// ABSTRACT CLASS FOR MATCHMAKING STRATEGY
//
class MatchmakingStrategy
{
public:
    virtual bool good_match(Player &p1, Player &p2) = 0;
    virtual void update_mmr(Player &winner, Player &loser, double actual_chances) = 0;
};

//
// NAIVE MATCHMAKING STRATEGY
//
class Naive_strategy : public MatchmakingStrategy
{
    const int offset = 10; // how much MMR changes for win/loss

public:
    // Checks if the match between players would be a good based on MMR
    // More complicated version would take into account search time, latency, etc.
    bool good_match(Player &p1, Player &p2)
    {
        return std::abs(p1.mmr - p2.mmr) < offset * 5;
    }

    // Updates MMR
    void update_mmr(Player &winner, Player &loser, double actual_chances)
    {
        winner.opponent_history.push_back(loser.skill);
        winner.mmr_history.push_back(winner.mmr);
        loser.opponent_history.push_back(winner.skill);
        loser.mmr_history.push_back(loser.mmr);

        winner.predicted_chances.push_back(0.5);
        loser.predicted_chances.push_back(0.5);

        winner.mmr += offset;
        loser.mmr -= offset;
    }
};

//
// ELO MATCHMAKING STRATEGY
//
class ELO_strategy : public MatchmakingStrategy
{
    const double K = 10;

public:
    // Checks if the match between players would be a good based on MMR
    // More complicated version would take into account search time, latency, etc.
    bool good_match(Player &p1, Player &p2)
    {
        return std::abs(p1.mmr - p2.mmr) < 70.0; // 35 MMR → 55% ; 70 → 60% ; 120 → 66%; 191 → 75%
    }

    // Updates MMR for
    void update_mmr(Player &winner, Player &loser, double actual_chances)
    {
        winner.opponent_history.push_back(loser.skill);
        winner.mmr_history.push_back(winner.mmr);
        loser.opponent_history.push_back(winner.skill);
        loser.mmr_history.push_back(loser.mmr);

        // Chances of winning for the winner and loser. /400 is changed to 173. to use exp instead of pow(10,)
        double Ew = 1 / (1 + exp((loser.mmr - winner.mmr) / 173.718));
        double El = 1 - Ew;

        winner.predicted_chances.push_back(Ew);
        loser.predicted_chances.push_back(El);

        // printf("Winner (%f) | Loser (%f) | Expected: %f Actual: %f | +%f \n", winner.mmr, loser.mmr, Ew, actual_chances, K * El);
        winner.mmr += K * El;
        loser.mmr -= K * El;

        prediction_difference.push_back(std::abs(actual_chances - Ew));
    }
};

//
// SIMULATION CLASS
//
class Simulation
{
    std::default_random_engine RNG;
    std::normal_distribution<> skill_distribution;
    MatchmakingStrategy *strategy;

public:
    std::vector<Player> players;
    Simulation(MatchmakingStrategy &strat)
    {
        strategy = &strat;
        // Initialize RNG generation for players
        // std::default_random_engine new_RNG(std::chrono::steady_clock::now().time_since_epoch().count()); //fixed seed for now
        std::default_random_engine new_RNG(1);
        RNG = new_RNG;
        std::normal_distribution<> new_skill_distribution(2820 / 2.2, 800 / 2.2); // mean, std
        skill_distribution = new_skill_distribution;
    }

    // Adds `number` of players to the simulation
    void add_players(int number)
    {
        for (int i = 0; i < number; i++)
        {
            Player p(skill_distribution(RNG));
            players.push_back(p);
        }
    }

    // Removes `number` of players from the simulation (from the start of players)
    void remove_players(int number)
    {
        if (number >= players.size())
            players.clear();
        else
            players.erase(players.begin(), players.begin() + number);
    }

    // Returns a chance of player p1 winning (based on skill)
    double get_chance(Player &p1, Player &p2)
    {
        // This depends on how we have chosen to distribute skill
        double chance = 1 / (1 + exp((p2.skill - p1.skill) / 173.718)); // ELO points equivalent
        // printf("P1s: %f vs P2s: %f --> %f%% for P1 \n", p1.skill, p2.skill, 100 * chance);
        return chance;
    }

    void resolve_game(Player &p1, Player &p2)
    {
        // printf("Playing a game between #%p (%i MMR) and #%p (%i MMR)\n", &p1, p1.mmr, &p2, p2.mmr);
        double p1_chance = get_chance(p1, p2);
        if (RNG() % 10000 <= 10000 * p1_chance)
            strategy->update_mmr(p1, p2, p1_chance);
        else
            strategy->update_mmr(p2, p1, 1 - p1_chance);
    }

    // Runs the simulation for `number` of games
    void play_games(int number)
    {
        // bool found_opponent;
        int player, opponent;
        int games_played = 0;

        while (games_played < number)
        {
            // Pick a random player
            player = RNG() % players.size();

            // Pick a random opponent
            for (int tries = 0; tries < 10000; tries++)
            {
                opponent = RNG() % players.size();
                if (opponent == player) // we don't want the same player
                    continue;
                if (strategy->good_match(players[player], players[opponent]))
                {
                    resolve_game(players[player], players[opponent]);
                    games_played++;
                    break;
                }
            }
        }
    }
};

// Saves player data into a json file for additional analysis
// Extremely slow to create a string or write (e.g 250MB). C-extension for Python is much faster.
void save_player_data(const std::vector<Player> &players)
{
    std::string s = "[\n";

    // Get player data
    for (const Player &p : players)
    {
        s += str("{\"skill\": ", p.skill, ", \"mmr\": ", p.mmr, ", \"opponent_history\": ", str(p.opponent_history));
        s += str(", \"mmr_history\": ", str(p.mmr_history), ", \"predicted_chances\": ", str(p.predicted_chances), "},\n");
    }
    s.erase(s.end() - 2, s.end());
    s += "\n]";

    // Save data
    std::ofstream myfile("data.json");
    if (myfile.is_open())
    {
        myfile << s << std::endl;
        myfile.close();
    }
    else
    {
        print("Failed to save data to json!");
    }
}

std::vector<double> get_prediction_diff()
{
    return prediction_difference;
}

std::vector<Player> run_sim(int players, int iterations)
{
    Timeit t;
    ELO_strategy strategy;
    Simulation simulation(strategy);
    simulation.add_players(players);
    simulation.play_games(iterations);
    print("Pure simulation finished in", t.ms() / 1000, "seconds");
    return simulation.players;
}