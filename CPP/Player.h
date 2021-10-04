#pragma once

#include "mutils.h"

#include <vector>
#include <string>

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
    // Vector of opponent skills.
    std::vector<double> opponent_history;
    // MMR history
    std::vector<double> mmr_history;
    // Chances predicted by the matchmaker
    std::vector<double> predicted_chances;
    // Define player and assign him his skill value
    Player(double pskill)
    {
        skill = pskill;
    };
};

//Overload my str fuction for printing the player
inline std::string str(Player p)
{
    return str("Player(skill=", p.skill) + str(", mmr=", p.mmr) + str(", opponent_history=", p.opponent_history) + str(", mmr_history=", p.mmr_history, ")");
}