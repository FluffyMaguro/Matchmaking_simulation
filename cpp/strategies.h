#pragma once

#include "player.h"

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
    double offset = 17;
    double multiplier = 100;

public:
    Naive_strategy();
    Naive_strategy(double pK, double pMult);
    bool good_match(Player &p1, Player &p2);
    double update_mmr(Player &winner, Player &loser, double actual_chances);
};

//
// ELO MATCHMAKING STRATEGY
//
class ELO_strategy : public MatchmakingStrategy
{
    double K = 7;

public:
    ELO_strategy();
    ELO_strategy(double pK);
    bool good_match(Player &p1, Player &p2);
    double update_mmr(Player &winner, Player &loser, double actual_chances);
};

//
// Tweaked ELO MATCHMAKING STRATEGY
//
class Tweaked_ELO_strategy : public MatchmakingStrategy
{
public:
    double K = 2;
    double KK = 145;
    int game_div = 35;

    Tweaked_ELO_strategy(){};
    Tweaked_ELO_strategy(double pK, double pKK, int pgame_div);
    bool good_match(Player &p1, Player &p2);
    virtual double get_learning_coefficient(Player &player, Player &other_player);
    double update_mmr(Player &winner, Player &loser, double actual_chances);
};

//
// Tweaked2 ELO MATCHMAKING STRATEGY
//

// Similar to normal ELO strategy but different uncertainity calculation
class Tweaked2_ELO_strategy : public Tweaked_ELO_strategy
{

public:
    double K = 2;
    double KK = 100;
    int game_div = 56;
    double coef = 0.3;

    Tweaked2_ELO_strategy(double pK, double pKK, int pgame_div, double pcoef);
    double get_learning_coefficient(Player &player, Player &other_player) override;
};
