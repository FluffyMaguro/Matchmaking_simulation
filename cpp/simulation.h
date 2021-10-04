#pragma once

#include "player.h"
#include "strategies.h"

#include <chrono>
#include <random>

class Simulation
{
    std::default_random_engine m_RNG;
    std::normal_distribution<> m_skill_distribution;
    std::unique_ptr<MatchmakingStrategy> m_strategy;

public:
    std::vector<Player> players;
    std::vector<double> prediction_difference;
    std::vector<double> match_accuracy;

    Simulation(std::unique_ptr<MatchmakingStrategy> strat);
    void add_players(int number);
    void add_players(double number);
    void remove_players(int number);
    double get_chance(Player &p1, Player &p2);
    void resolve_game(Player &p1, Player &p2);
    void play_games(int number);
    void play_games(double number);
};
