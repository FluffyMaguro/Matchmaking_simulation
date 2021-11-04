#pragma once

#include "Python.h"

// Data structure used for storing player data in one match
struct match_pair
{
    double winner_mu;
    double winner_sigma;
    double loser_mu;
    double loser_sigma;
    int draw = 0; // 0 for non-draw; 1 for draw
};

PyObject *set_trueskill_rate_1v1(PyObject *dummy, PyObject *args);
match_pair trueskill_update(match_pair pair);