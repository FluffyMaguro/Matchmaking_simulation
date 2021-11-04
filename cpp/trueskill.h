#pragma once

struct match_pair
{
    double winner_mu;
    double winner_sigma;
    double loser_mu;
    double loser_sigma;
    int draw = 0; // 0 for non-draw; 1 for draw
};

// from sim.cpp
match_pair trueskill_update(match_pair pair);