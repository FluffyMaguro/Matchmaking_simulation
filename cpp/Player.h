#pragma once

#include "mutils.h"

#include <vector>
#include <memory>

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
    // Uncertainity
    double sigma = 0;
    // Vector of opponent skills.
    std::unique_ptr<std::vector<double>> opponent_history;
    // MMR history
    std::unique_ptr<std::vector<double>> mmr_history;
    // Chances predicted by the matchmaker
    std::unique_ptr<std::vector<double>> predicted_chances;
    // How sigma changes
    std::unique_ptr<std::vector<double>> sigma_history;

    // Define player and assign him his skill value
    Player(double pskill)
    {
        skill = pskill;
        opponent_history.reset(new std::vector<double>);
        mmr_history.reset(new std::vector<double>);
        predicted_chances.reset(new std::vector<double>);
        sigma_history.reset(new std::vector<double>);
    }
};