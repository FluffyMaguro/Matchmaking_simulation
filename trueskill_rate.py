from typing import Tuple

import trueskill


def rate_1v1(winner_mu: float,
             winner_sigma: float,
             loser_mu: float,
             loser_sigma: float,
             draw: bool = False) -> Tuple[float, float, float, float]:
    try:
        winner = trueskill.Rating(mu=winner_mu, sigma=winner_sigma)
        loser = trueskill.Rating(mu=loser_mu, sigma=loser_sigma)

        # print("PYTHON BEFORE :", winner_mu, winner_sigma, loser_mu,
        #       loser_sigma, bool(draw))

        winner, loser = trueskill.rate_1vs1(winner, loser, bool(draw))

        # print("PYTHON AFTER :", winner.mu, winner.sigma, loser.mu, loser.sigma)
    except Exception as e:
        print(e)
        return (winner_mu, winner_sigma, loser_mu, loser_sigma)

    return (winner.mu, winner.sigma, loser.mu, loser.sigma)
