#include "player.h"
#include "strategies.h"
#include "simulation.h"

#include <chrono>
#include <random>
#include <memory>

Simulation::Simulation(std::unique_ptr<MatchmakingStrategy> strat)
{
    // Initialize ENG
    int seed = static_cast<int>(std::chrono::steady_clock::now().time_since_epoch().count());
    m_RNG.seed(seed);

    // Move pointer from argument unique smart pointer to strategy
    m_strategy = std::move(strat);
    // Create those vectors on the heap
    prediction_difference.reset(new std::vector<double>);
    match_accuracy.reset(new std::vector<double>);
}

// Adds `number` of players to the simulation
void Simulation::add_players(int number)
{
    for (int i = 0; i < number; i++)
        players.push_back(Player(m_skill_distribution(m_RNG)));
}
void Simulation::add_players(double number)
{
    add_players(static_cast<int>(number));
}

// Removes `number` of players from the simulation (from the start of players)
void Simulation::remove_players(int number)
{
    if (number >= players.size())
        players.clear();
    else
        players.erase(players.begin(), players.begin() + number);
}

// Returns a chance of player p1 winning (based on skill)
double Simulation::get_chance(Player &p1, Player &p2)
{
    // This depends on how we have chosen to distribute skill
    return 1 / (1 + exp((p2.skill - p1.skill) / 173.718)); // ELO points equivalent
}

void Simulation::resolve_game(Player &p1, Player &p2)
{
    double p1_chance = get_chance(p1, p2);
    match_accuracy->push_back(std::abs(p1_chance - 0.5));
    double pred_diff;
    if (m_RNG() % 10000 <= 10000 * p1_chance)
        pred_diff = m_strategy->update_mmr(p1, p2, p1_chance);
    else
        pred_diff = m_strategy->update_mmr(p2, p1, 1 - p1_chance);

    try
    {
        prediction_difference->push_back(pred_diff);
    }
    catch (...)
    {
        print("Failed to pushback to prediction_difference. Size:", prediction_difference->size());
    }
}

// Runs the simulation for `number` of games
void Simulation::play_games(int number)
{
    // bool found_opponent;
    int player, opponent;
    int games_played = 0;
    int players_num = static_cast<int>(players.size());

    while (games_played < number)
    {
        // Pick a random player
        player = m_RNG() % players_num;

        // Pick a random opponent
        for (int tries = 0; tries < 10000; tries++)
        {
            opponent = m_RNG() % players_num;
            if (opponent == player) // we don't want the same player
                continue;
            if (m_strategy->good_match(players[player], players[opponent]))
            {
                resolve_game(players[player], players[opponent]);
                games_played++;
                break;
            }
        }
    }
}

void Simulation::play_games(double number)
{
    play_games(static_cast<int>(number));
}
