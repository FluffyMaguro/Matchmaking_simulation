import traceback
from functools import lru_cache
from typing import Tuple

import trueskill


class Stats:
    min_mu = 0
    max_mu = 0


@lru_cache()  # Minimal decrease if any
def rate_1v1(winner_mu: float,
             winner_sigma: float,
             loser_mu: float,
             loser_sigma: float,
             draw: bool = False) -> Tuple[float, float, float, float]:
    """
    Convert player parameters into trueskill ratings, update them according to the algorithm
    and send back again as tuple of floats

    """

    try:
        winner = trueskill.Rating(mu=winner_mu, sigma=winner_sigma)
        loser = trueskill.Rating(mu=loser_mu, sigma=loser_sigma)
        winner, loser = trueskill.rate_1vs1(winner, loser, bool(draw))

        if winner.mu > Stats.max_mu:
            Stats.max_mu = winner.mu
        if loser.mu < Stats.min_mu:
            Stats.min_mu = loser.mu

    except Exception:
        traceback.print_exc()
        return (winner_mu, winner_sigma, loser_mu, loser_sigma)
    return (winner.mu, winner.sigma, loser.mu, loser.sigma)
