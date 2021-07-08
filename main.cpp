#include <iostream>
#include <string>
#include <vector>
// #include <cstdlib>
// #include <chrono>
#include <random>

#include "mutils.h" // adds "print" and "str" fuction templates

//
// PLAYER CLASS
//
class Player
{
public:
    // Actual player skill
    double skill;
    // MMR assigned by the matchmaker
    int mmr = 0;
    // Vector of opponent skills. It's used to define the matchmaker accuracy.
    std::vector<double> history;
    // Define player and assign him his skill value
    Player(double pskill)
    {
        skill = pskill;
    };
};

//Overload my str fuction for printing the player
std::string str(Player p)
{
    return str("Player(skill=", p.skill) + str(", mmr=", p.mmr) + str(", history=", p.history, ")");
}

// ABSTRACT CLASS FOR MATCHMAKING STRATEGY
class MatchmakingStrategy
{
public:
    virtual void get_chance(Player p1, Player p2) = 0;
    virtual void update_mmr(Player p1, Player p2) = 0;
};

//
// ELO MATCHMAKING STRATEGY
//
class ELO_strategy : public MatchmakingStrategy
{
public:
    void get_chance(Player p1, Player p2)
    {
    }
    void update_mmr(Player p1, Player p2)
    {
    }
};

//
// SIMULATION CLASS
//
class Simulation
{
    std::default_random_engine generator;
    std::normal_distribution<> skill_distribution;
    MatchmakingStrategy *strategy;
    std::vector<Player> players;

public:
    Simulation(MatchmakingStrategy &strat)
    {
        strategy = &strat;
        // Initialize RNG generation for players
        std::default_random_engine new_generator(333); //fixed seed for now
        generator = new_generator;
        std::normal_distribution<> new_skill_distribution(2, 0.4); // mean, std
        skill_distribution = new_skill_distribution;
    }

    // Adds `number` of players to the simulation
    void add_players(int number)
    {
        for (int i = 0; i < number; i++)
        {
            Player p(skill_distribution(generator));
            players.push_back(p);
            print("Adding:", p);
        }
    }

    // Removes `number` of players from the simulation
    void remove_players(int number)
    {
        for (int i = 0; i < number && players.size() > 0; i++)
        {
            players.pop_back();
        }
    }

    // Runs the simulation for `number` of games
    void play_games(int number)
    {
        for (int i = 0; i < number; i++)
        {
            print("playing a game...");
            /*
            1. find a match (based on assiged MMR)
            2. resolve match (randomly, based on true skill)
            3. update mmr, add history
            */
        }
    }

    // Calculates inaccuracy of matchmaking (sum of square of skill differences / number of players)
    // Accurate only for the same amount of games (as early games will be more inaccurate)
    double get_inaccuracy()
    {
        double total = 0;
        for (const Player &player : players)
        {
            for (const double &opponent : player.history)
            {
                double diff = opponent - player.skill;
                total += diff * diff;
            }
        }
        return total / players.size();
    }
};

int main()
{
    ELO_strategy strategy;
    Simulation simulation(strategy);
    simulation.add_players(10);
    simulation.play_games(10);
    double inaccuracy = simulation.get_inaccuracy();
    print("inaccuracy:", inaccuracy);
}