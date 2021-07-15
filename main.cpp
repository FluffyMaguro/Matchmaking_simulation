#include <iostream>
#include <string>
#include <vector>
// #include <cstdlib>
#include <chrono>
#include <random>
#include <cmath>

#include "mutils.h" // adds "print" and "str" fuction templates

std::vector<double> chance_differences;
//
// PLAYER CLASS
//
class Player
{
public:
    // Actual player skill
    double skill;
    // MMR assigned by the matchmaker
    double mmr = 2000;
    // Vector of opponent skills. It's used to define the matchmaker accuracy.
    std::vector<double> opponent_history;
    //
    std::vector<double> mmr_history;
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
class naive_strategy : public MatchmakingStrategy
{
    const int offset = 1; // how much MMR changes for win/loss

public:
    // Checks if the match between players would be a good based on MMR
    // More complicated version would take into account search time, latency, etc.
    bool good_match(Player &p1, Player &p2)
    {
        return std::abs(p1.mmr - p2.mmr) < offset * 2;
    }

    // Updates MMR
    void update_mmr(Player &winner, Player &loser, double actual_chances)
    {
        winner.opponent_history.push_back(loser.skill);
        winner.mmr_history.push_back(winner.mmr);
        loser.opponent_history.push_back(winner.skill);
        loser.mmr_history.push_back(loser.mmr);

        winner.mmr += offset;
        loser.mmr -= offset;
    }
};

//
// ELO MATCHMAKING STRATEGY
//
class ELO_strategy : public MatchmakingStrategy
{
    // ** TEST WITH fixed K
    // ** TEST WITH K(player.mmr)
    // ** TRY CHANGING So diff in chances is more aggressive (more points if chances are skewed more?)
  
    const double K = 32;

public:
    // Checks if the match between players would be a good based on MMR
    // More complicated version would take into account search time, latency, etc.
    bool good_match(Player &p1, Player &p2)
    {
        return std::abs(p1.mmr - p2.mmr) < 35.0; // 35 MMR → 55% ; 70 → 60% ; 120 → 66%; 191 → 75%
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

        // printf("Winner (%f) | Loser (%f) | Expected: %f Actual: %f | +%f \n", winner.mmr, loser.mmr, Ew, actual_chances, K * El);
        winner.mmr += K * El;
        loser.mmr -= K * El;

        
        double diff = Ew - actual_chances;
        chance_differences.push_back(diff*diff);
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
        std::normal_distribution<> new_skill_distribution(1, 0.4); // mean, std
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
        double chance = 1 / (1 + exp(-10 * (p1.skill - p2.skill)));
        // printf("P1s: %f vs P2s: %f --> %f%% for P1 \n", p1.skill, p2.skill, 100 * chance);
        return chance;
    }

    void resolve_game(Player &p1, Player &p2)
    {
        // printf("Playing a game between #%p (%i MMR) and #%p (%i MMR)\n", &p1, p1.mmr, &p2, p2.mmr);

        if (RNG() % 10000 <= 10000 * get_chance(p1, p2))
            strategy->update_mmr(p1, p2, get_chance(p1, p2));
        else
            strategy->update_mmr(p2, p1, get_chance(p2, p1));
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
            // found_opponent = false;

            // Pick a random opponent
            for (int tries = 0; tries < 10000; tries++)
            {
                opponent = RNG() % players.size();
                if (opponent == player) // we don't want the same player
                    continue;
                if (strategy->good_match(players[player], players[opponent]))
                {
                    resolve_game(players[player], players[opponent]);
                    // found_opponent = true;
                    games_played++;
                    break;
                }
            }
            // If we didn't found a good opponent
            // if (!found_opponent)
            // {
            //     printf("failed to find a good opponent (%f MMR)\n", players[player].mmr);
            // }
        }
    }

    // Calculates inaccuracy of matchmaking (sum of square of skill differences / number of players)
    // Accurate only for the same amount of games (as early games will be more inaccurate)
    double get_inaccuracy()
    {
        double total = 0;
        for (const Player &player : players)
        {
            for (const double &opponent : player.opponent_history)
            {
                double diff = opponent - player.skill;
                total += diff * diff;
            }
        }
        return total / players.size();
    }
};

// Saves player data into a json file for additional analysis
void save_player_data(const std::vector<Player> &players)
{
    std::string s = "[\n";

    // Get player data
    auto t1 = std::chrono::high_resolution_clock::now();

    for (const Player &p : players)
    {
        s += str("{\"skill\": ", p.skill, ", \"mmr\": ", p.mmr, ", \"opponent_history\": ", str(p.opponent_history));
        s += str(", \"mmr_history\": ", str(p.mmr_history), "},\n");
    }
    s.erase(s.end() - 2, s.end());
    s += "\n]";

    auto t2 = std::chrono::high_resolution_clock::now();
    std::chrono::duration<double, std::milli> ms_double = t2 - t1;
    print("String created", ms_double.count(), "ms");

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

int main()
{
    ELO_strategy strategy;
    Simulation simulation(strategy);
    simulation.add_players(200);
    simulation.play_games(100000);

    double inaccuracy = simulation.get_inaccuracy();
    print("Inaccuracy:", inaccuracy);
    /*
    Naive 200-100000 (players, games)
    limit = 1*offset → Inaccuracy: 6.00
    limit = 2*offset → Inaccuracy: 6.67
    limit = 5*offset → Inaccuracy: 8.6
    limit = 10*offset → Inaccuracy: 12
    limit = 100*offset → Inaccuracy: 60

    ELO 200-100000
    K       Opponents       Inaccuracy
    4       99              19.2
    8       85              12.5
    16      75              9.0
    32      69.4            7.5
    64      67.4            6.71
    128     66.6            6.17

    */

    // Let's try analysis in Python

    save_player_data(simulation.players);
    system("python analyse.py");

    double total = 0;
    for (const double &i : chance_differences) {
        total += i;
    }
    print("sum of chance_differences^2:",total);
}