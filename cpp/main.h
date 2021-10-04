#pragma once

#include "simulation.h"

#include <memory>

std::unique_ptr<Simulation> run_sim(int players, int iterations, int sp1, int sp2, int sp3, double sp4, const std::string &strategy_type, bool gradual = false);
