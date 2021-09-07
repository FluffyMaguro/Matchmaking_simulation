#include <iostream>
#include <string>
#include <vector>
#include <chrono>
#include <random>
#include <cmath>
#include <algorithm>
#include <memory>

#include "mutils.h" // random helper functions

//
// PLAYER CLASS
// I could be more memory efficient if I save histories to a Game class instead of players.
// Right now there is a duplication of data (for two players)
//
class Player
{
public:
    // Actual player skill
    double skill;
    // MMR assigned by the matchmaker
    double mmr = 2820 / 2.2;
    // Vector of opponent skills.
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
    virtual double update_mmr(Player &winner, Player &loser, double actual_chances) = 0;
};

//
// NAIVE MATCHMAKING STRATEGY
//
class Naive_strategy : public MatchmakingStrategy
{
    double offset = 17; // How much MMR changes for win/loss
    // Multiplier*offset decides whether two player MMR difference is still a good match.
    // Optimal is very low value (3), but that's unrealistic for getting matches relatively quickly.
    double multiplier = 100;

public:
    Naive_strategy()
    {
        std::cout << "NAIVE strategy (" << offset << ", " << multiplier << ")";
    }
    Naive_strategy(double pK, double pMult)
    {
        if (pK != -1)
        {
            offset = pK;
        }
        if (pMult != -1)
        {
            multiplier = pMult;
        }
        std::cout << "NAIVE strategy (" << offset << ", " << multiplier << ")";
    }
    // Checks if the match between players would be a good based on MMR
    // More complicated version would take into account search time, latency, etc.
    bool good_match(Player &p1, Player &p2)
    {
        return std::abs(p1.mmr - p2.mmr) < offset * multiplier;
    }

    // Updates MMR
    double update_mmr(Player &winner, Player &loser, double actual_chances)
    {
        winner.opponent_history.push_back(loser.skill);
        winner.mmr_history.push_back(winner.mmr);
        loser.opponent_history.push_back(winner.skill);
        loser.mmr_history.push_back(loser.mmr);

        winner.mmr += offset;
        loser.mmr -= offset;
        return std::abs(actual_chances - 0.5);
    }
};

//
// ELO MATCHMAKING STRATEGY
//
class ELO_strategy : public MatchmakingStrategy
{
    double K = 7;

public:
    ELO_strategy()
    {
        std::cout << "ELO strategy (" << K << ")";
    }
    ELO_strategy(double pK)
    {
        if (pK != -1)
        {
            K = pK;
        }
        std::cout << "ELO strategy (" << K << ")";
    }
    // Checks if the match between players would be a good based on MMR
    // More complicated version would take into account search time, latency, etc.
    bool good_match(Player &p1, Player &p2)
    {
        return std::abs(p1.mmr - p2.mmr) < 120.0; // 35 MMR → 55% ; 70 → 60% ; 120 → 66%; 191 → 75%
    }

    // Updates MMR for
    double update_mmr(Player &winner, Player &loser, double actual_chances)
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

        return std::abs(actual_chances - Ew);
    }
};

//
// Tweaked ELO MATCHMAKING STRATEGY
//
class Tweaked_ELO_strategy : public MatchmakingStrategy
{
    double K = 6;
    double KK = 135;
    int game_div = 15;

public:
    Tweaked_ELO_strategy()
    {
        std::cout << "Tweaked_ELO strategy (" << K << ", " << KK << ", " << game_div << ")";
    }
    Tweaked_ELO_strategy(double pK, double pKK, int pgame_div)
    {
        if (pK != -1)
        {
            K = pK;
        }
        if (pKK != -1)
        {
            KK = pKK;
        }
        if (pgame_div != -1)
        {
            game_div = pgame_div;
        }
        std::cout << "Tweaked_ELO strategy (" << K << ", " << KK << ", " << game_div << ")";
    }
    // Checks if the match between players would be a good based on MMR
    // More complicated version would take into account search time, latency, etc.
    bool good_match(Player &p1, Player &p2)
    {
        return std::abs(p1.mmr - p2.mmr) < 120.0; // 35 MMR → 55% ; 70 → 60% ; 120 → 66%; 191 → 75%
    }

    // Updates MMR for
    double update_mmr(Player &winner, Player &loser, double actual_chances)
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

        // Simply update coeficient based on number of games
        // Fewer games → faster update
        // More games → slower update
        int winner_games = static_cast<int>(winner.opponent_history.size()) - 1;
        int loser_games = static_cast<int>(winner.opponent_history.size()) - 1;

        double winner_coef = K + KK * exp(-winner_games / game_div);
        double loser_coef = K + KK * exp(-loser_games / game_div);
        winner.mmr += winner_coef * El;
        loser.mmr -= loser_coef * El;

        return std::abs(actual_chances - Ew);
    }
};

//
// SIMULATION CLASS
//
class Simulation
{
    std::default_random_engine RNG;
    std::normal_distribution<> skill_distribution;
    std::unique_ptr<MatchmakingStrategy> strategy;

public:
    std::vector<Player> players;
    std::vector<double> prediction_difference;
    std::vector<double> match_accuracy;

    Simulation(std::unique_ptr<MatchmakingStrategy> strat)
    {
        strategy = std::move(strat); // Move pointer from argument unique smart pointer to strategy
        int seed = static_cast<int>(std::chrono::steady_clock::now().time_since_epoch().count());
        RNG.seed(seed);
        std::normal_distribution<> new_skill_distribution(2820 / 2.2, 800 / 2.2); // mean, std
        skill_distribution = new_skill_distribution;
    }

    ~Simulation()
    {
        // delete strategy; //not necessary since we are using unique_ptr
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
    void add_players(double number)
    {
        add_players(static_cast<int>(number));
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
        return 1 / (1 + exp((p2.skill - p1.skill) / 173.718)); // ELO points equivalent
    }

    void resolve_game(Player &p1, Player &p2)
    {
        double p1_chance = get_chance(p1, p2);
        match_accuracy.push_back(std::abs(p1_chance - 0.5));
        double pred_diff;
        if (RNG() % 10000 <= 10000 * p1_chance)
            pred_diff = strategy->update_mmr(p1, p2, p1_chance);
        else
            pred_diff = strategy->update_mmr(p2, p1, 1 - p1_chance);

        try
        {
            prediction_difference.push_back(pred_diff);
        }
        catch (...)
        {
            print("Failed to pushback to prediction_difference. Size:", prediction_difference.size());
        }
    }

    // Runs the simulation for `number` of games
    void play_games(int number)
    {
        // bool found_opponent;
        int player, opponent;
        int games_played = 0;
        int players_num = players.size();

        while (games_played < number)
        {
            // Pick a random player
            player = RNG() % players_num;

            // Pick a random opponent
            for (int tries = 0; tries < 10000; tries++)
            {
                opponent = RNG() % players_num;
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

    void play_games(double number)
    {
        play_games(static_cast<int>(number));
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

// Creates simulation, runs it, and returns a reference to it
std::unique_ptr<Simulation> run_sim(int players, int iterations, int sp1, int sp2, int sp3, std::string strategy_type, bool gradual = false)
{
    Timeit t;
    std::unique_ptr<MatchmakingStrategy> strategy;
    if ((strategy_type == "tweaked_elo") || (strategy_type == "default"))
        strategy = std::make_unique<Tweaked_ELO_strategy>(sp1, sp2, sp3);
    else if (strategy_type == "elo")
        strategy = std::make_unique<ELO_strategy>(sp1);
    else if (strategy_type == "naive")
        strategy = std::make_unique<Naive_strategy>(sp1, sp2);
    else
        print("ERROR: Invalid strategy type!!!");

    std::unique_ptr<Simulation> sim = std::make_unique<Simulation>(std::move(strategy));
    if (gradual)
    {
        std::cout << " | Add players gradually" << std::endl;
        sim->add_players(players * 0.5);
        sim->play_games(iterations * 0.4);
        sim->add_players(players * 0.3);
        sim->play_games(iterations * 0.4);
        sim->add_players(players * 0.2);
        sim->play_games(iterations * 0.2);
    }
    else
    {
        std::cout << " | Add all players at once" << std::endl;
        sim->add_players(players);
        sim->play_games(iterations);
    }

    print("Simulation finished in", t.s(), "seconds");
    return sim;
}
