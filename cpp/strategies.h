#pragma once

#include "player.h"
#include "trueskill.h"

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

//
// TRUESKILL STRATEGY
//
class Trueskill_strategy : public MatchmakingStrategy
{

public:
    Trueskill_strategy()
    {
        std::cout << "TRUESKILL strategy\n";
    }

    bool good_match(Player &p1, Player &p2)
    {
        // TEMPORARY, WILL CALCULATE IT HERE DIRECTLY
        return true;
    }
    double update_mmr(Player &winner, Player &loser, double actual_chances)
    {
        winner.opponent_history->push_back(loser.skill);
        winner.mmr_history->push_back(winner.mmr);
        loser.opponent_history->push_back(winner.skill);
        loser.mmr_history->push_back(loser.mmr);

        // Update player skill and sigma
        match_pair old_pair{winner.mmr, winner.sigma, loser.mmr, loser.sigma};
        match_pair new_pair = trueskill_update(old_pair);

        winner.mmr = new_pair.winner_mu;
        winner.sigma = new_pair.winner_sigma;
        loser.mmr = new_pair.loser_mu;
        loser.sigma = new_pair.loser_sigma;

        // Return the difference between actual and predicted chances
        // TEMPORARY
        return std::abs(actual_chances - 0.5);
    }
};