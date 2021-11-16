#include "mutils.h"
#include "player.h"
#include "strategies.h"
#include "simulation.h"
#include "main.h"

#include <iostream>
#include <string>
#include <memory>

// Creates simulation, runs it, and returns a reference to it
Simulation run_sim(int players, int iterations, int sp1, int sp2, int sp3, double sp4, const std::string &strategy_type, bool gradual)
{
    Timeit t;
    std::unique_ptr<MatchmakingStrategy> strategy;
    if ((strategy_type == "tweaked_elo") || (strategy_type == "default"))
        strategy = std::make_unique<Tweaked_ELO_strategy>(sp1, sp2, sp3);
    else if (strategy_type == "tweaked2_elo")
        strategy = std::make_unique<Tweaked2_ELO_strategy>(sp1, sp2, sp3, sp4);
    else if (strategy_type == "elo")
        strategy = std::make_unique<ELO_strategy>(sp1);
    else if (strategy_type == "naive")
        strategy = std::make_unique<Naive_strategy>(sp1, sp2);
    else if (strategy_type == "trueskill")
        strategy = std::make_unique<Trueskill_strategy>();
    else
        print("ERROR: Invalid strategy type!!!");

    Simulation sim = Simulation(std::move(strategy));

    // For Trueskill we will want different default player parameters
    if (strategy_type == "trueskill")
    {
        sim.m_force_player_mmr = 25.0;
        sim.m_force_player_sigma = 25. / 3;
    }

    if (gradual)
    {
        std::cout << "Add players gradually" << std::endl;
        sim.add_players(players * 0.5);
        sim.play_games(iterations * 0.4);
        sim.add_players(players * 0.3);
        sim.play_games(iterations * 0.4);
        sim.add_players(players * 0.2);
        sim.play_games(iterations * 0.2);
    }
    else
    {
        std::cout << "Add all players at once" << std::endl;
        sim.add_players(players);
        sim.play_games(iterations);
    }

    print("Simulation finished in", t.s(), "seconds");

    // I can just return the class. The compiler will do return-value-optimization
    // and correctly move this object into a new variable without copying.
    return sim;
}
