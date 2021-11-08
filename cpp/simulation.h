#pragma once

#include "player.h"
#include "strategies.h"

#include <chrono>
#include <random>
#include <memory>

class Simulation
{
    std::default_random_engine m_RNG;
    std::normal_distribution<> m_skill_distribution = std::normal_distribution<>(2820 / 2.2, 800 / 2.2);
    std::unique_ptr<MatchmakingStrategy> m_strategy;

public:
    std::vector<Player> players;
    // The difference between actual winning chances and predicted winning chances
    std::unique_ptr<std::vector<double>> prediction_difference;
    // How far the chosen player winning chance is from 50%
    std::unique_ptr<std::vector<double>> match_accuracy;
    // The percent of players that are considered a good match for the first chosen player
    std::unique_ptr<std::vector<double>> good_match_fraction;
    double m_force_player_mmr = -1.0;
    double m_force_player_sigma = -1.0;

    Simulation(std::unique_ptr<MatchmakingStrategy> strat);
    void add_players(int number);
    void add_players(double number);
    void remove_players(int number);
    double get_chance(Player &p1, Player &p2);
    void resolve_game(Player &p1, Player &p2);
    void play_games(int number);
    void play_games(double number);
    void calculate_good_match_fraction(Player &p, int players_num);
};
