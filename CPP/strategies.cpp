#include "strategies.h"

#include <iostream>

//
// NAIVE MATCHMAKING STRATEGY
//

Naive_strategy::Naive_strategy()
{
    std::cout << "NAIVE strategy (" << offset << ", " << multiplier << ")\n";
}

Naive_strategy::Naive_strategy(double pK, double pMult)
{
    if (pK != -1)
        offset = pK;
    if (pMult != -1)
        multiplier = pMult;
    std::cout << "NAIVE strategy (" << offset << ", " << multiplier << ")\n";
}
// Checks if the match between players would be a good based on MMR
// More complicated version would take into account search time, latency, etc.
bool Naive_strategy::good_match(Player &p1, Player &p2)
{
    return std::abs(p1.mmr - p2.mmr) < offset * multiplier;
}

double Naive_strategy::update_mmr(Player &winner, Player &loser, double actual_chances)
{
    winner.opponent_history.push_back(loser.skill);
    winner.mmr_history.push_back(winner.mmr);
    loser.opponent_history.push_back(winner.skill);
    loser.mmr_history.push_back(loser.mmr);

    winner.mmr += offset;
    loser.mmr -= offset;
    return std::abs(actual_chances - 0.5);
}

//
// ELO MATCHMAKING STRATEGY
//

ELO_strategy::ELO_strategy()
{
    std::cout << "ELO strategy (" << K << ")\n";
}
ELO_strategy::ELO_strategy(double pK)
{
    if (pK != -1)
        K = pK;
    std::cout << "ELO strategy (" << K << ")\n";
}
// Checks if the match between players would be a good based on MMR
// More complicated version would take into account search time, latency, etc.
bool ELO_strategy::good_match(Player &p1, Player &p2)
{
    return std::abs(p1.mmr - p2.mmr) < 120.0; // 35 MMR → 55% ; 70 → 60% ; 120 → 66%; 191 → 75%
}

// Updates MMR for
double ELO_strategy::update_mmr(Player &winner, Player &loser, double actual_chances)
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

    winner.mmr += K * El;
    loser.mmr -= K * El;

    return std::abs(actual_chances - Ew);
}

//
// Tweaked ELO MATCHMAKING STRATEGY
//

Tweaked_ELO_strategy::Tweaked_ELO_strategy(double pK, double pKK, int pgame_div)
{
    if (pK != -1)
        K = pK;
    if (pKK != -1)
        KK = pKK;
    if (pgame_div != -1)
        game_div = pgame_div;
    std::cout << "Tweaked_ELO strategy (" << K << ", " << KK << ", " << game_div << ")\n";
}

// Checks if the match between players would be a good based on MMR
// More complicated version would take into account search time, latency, etc.
bool Tweaked_ELO_strategy::good_match(Player &p1, Player &p2)
{
    return std::abs(p1.mmr - p2.mmr) < 120.0; // 35 MMR → 55% ; 70 → 60% ; 120 → 66%; 191 → 75%
}

// Returns a learning coefficient for the player
double Tweaked_ELO_strategy::get_learning_coefficient(Player &player, Player &other_player)
{
    double games = static_cast<double>(player.opponent_history.size()) - 1.0;
    return exp(-games / game_div);
}

// Updates MMR for
double Tweaked_ELO_strategy::update_mmr(Player &winner, Player &loser, double actual_chances)
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
    double winner_coef = K + KK * get_learning_coefficient(winner, loser);
    double loser_coef = K + KK * get_learning_coefficient(loser, winner);
    winner.mmr += winner_coef * El;
    loser.mmr -= loser_coef * El;

    return std::abs(actual_chances - Ew);
}

//
// Tweaked2 ELO MATCHMAKING STRATEGY
//

Tweaked2_ELO_strategy::Tweaked2_ELO_strategy(double pK, double pKK, int pgame_div, double pcoef)
{
    if (pK != -1)
        K = pK;
    if (pKK != -1)
        KK = pKK;
    if (pgame_div != -1)
        game_div = pgame_div;
    if (pcoef != -1)
        coef = pcoef;
    std::cout << "Tweaked2_ELO strategy (" << K << ", " << KK << ", " << game_div << ", " << coef << ")\n";
}

// Returns uncertainity for the player
double Tweaked2_ELO_strategy::get_learning_coefficient(Player &player, Player &other_player)
{
    // The idea here learning lowers as the player gets more games
    // And playing a new opponent will give you lower learning coefficient (wont lose too many points to him)
    // But a new player playing an old player gets high learning coefficient (still can gain a lot of points by playing someone solid)
    int player_games = static_cast<int>(player.opponent_history.size()) - 1;
    int other_player_games = static_cast<int>(other_player.opponent_history.size()) - 1;
    return std::min(exp((-coef * other_player_games - player_games) / game_div), 1.0);
}