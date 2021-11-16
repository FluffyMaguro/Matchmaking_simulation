#pragma once

#include "player.h"
#include "trueskill.h"
#include <cmath>

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
    const double MU = 25.;          //   25
    const double SIGMA = 25. / 3;    //  ~ 8.333
    const double BETA = 27. / 6;  //  ~ 4.166    # Variance of performance
    const double TAU = SIGMA / 100; //  ~ 0.083    # Dynamic variance

public:
    Trueskill_strategy()
    {
        std::cout << "TRUESKILL strategy\n";
    }

    double match_quality(Player &p1, Player &p2)
    {
        /* Calculates relative probability of draw between to players relative to probability of a draw between
        two equally skilled players (when draw_margin approaching 0). So it's always between 0 and 1.
        This is what trueskill match quality returns. For the best match this would be maximized.
        That equals the highest chance for a draw regardless of actual draw chance in given game. 

        Values between 0-1 and two players with default settings leads to 0.4472 quality */
        double sigma2 = 2 * pow(BETA, 2) + pow(p1.sigma, 2) + pow(p2.sigma, 2);
        double sqrt_part = sqrt((2 * pow(BETA, 2)) / sigma2);
        double exp_part = exp(-1 * pow(p1.mmr - p2.mmr, 2) / (2 * sigma2));
        return sqrt_part * exp_part;
    }

    bool good_match(Player &p1, Player &p2)
    {
        // 0.40 since default settings lead to 0.4472 quality
        return match_quality(p1, p2) > 0.40 && std::abs(winning_chance(p1, p2) - 0.5) < 0.17;
    }

    // Calculates normal distribution at point
    double normalCDF(double value, double mu, double sigma)
    {
        const double sqrt2 = sqrt(2);
        return 0.5 + 0.5 * erf((value - mu) / (sigma * sqrt2));
    }

    // Calculates the winning chance of player p1
    double winning_chance(Player &p1, Player &p2, double draw_margin = 0)
    {
        double mu = p1.mmr - p2.mmr;
        double sigma = sqrt(pow(p1.sigma, 2.0) + pow(p2.sigma, 2.0) + 2.0 * pow(BETA, 2.0));
        return 1.0 - normalCDF(draw_margin, mu, sigma);
    }

    double update_mmr(Player &winner, Player &loser, double actual_chances)
    {
        winner.opponent_history->push_back(loser.skill);
        winner.mmr_history->push_back(winner.mmr);
        loser.opponent_history->push_back(winner.skill);
        loser.mmr_history->push_back(loser.mmr);
        winner.sigma_history->push_back(winner.sigma);
        loser.sigma_history->push_back(loser.sigma);

        // Update player skill and sigma
        match_pair old_pair{winner.mmr, winner.sigma, loser.mmr, loser.sigma};
        match_pair new_pair = trueskill_update(old_pair);

        double p1_winning_chance = winning_chance(winner, loser);
        winner.predicted_chances->push_back(p1_winning_chance);
        loser.predicted_chances->push_back(1 - p1_winning_chance);
        // if (p1_winning_chance > 0.9)
        //     print("winning chance:", p1_winning_chance, "match quality:", match_quality(winner, loser), winner.mmr, winner.sigma, loser.mmr, loser.sigma);

        winner.mmr = new_pair.winner_mu;
        winner.sigma = new_pair.winner_sigma;
        loser.mmr = new_pair.loser_mu;
        loser.sigma = new_pair.loser_sigma;

        // Return the difference between actual and predicted chances
        return std::abs(actual_chances - p1_winning_chance);
    }
};