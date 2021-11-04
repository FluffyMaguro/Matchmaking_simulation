import math
from statistics import NormalDist
from typing import Tuple

import matplotlib.pyplot as plt
import numpy as np
import trueskill

MU = 25.  #   25
SIGMA = 25 / 3  #  ~ 8.333
BETA = SIGMA / 2  #  ~ 4.166    # Variance of performance


def match_outcomes(p1: trueskill.Rating,
                   p2: trueskill.Rating,
                   draw_margin: int = 0) -> Tuple[float, float, float]:
    """ Calculates the chance of winning and drawing for players
    
    args:
        p1: player 01
        p2: player 02
        draw_margin: how many points from zero would the game be considered a draw

    returns:
        tuple(p1_winning_chance, draw_chance, p2_winning_chance)
    
    """
    if draw_margin < 0:
        raise ValueError("draw_margin cannot be negative")

    mu = p1.mu - p2.mu
    sigma = math.sqrt(p1.sigma**2 + p2.sigma**2 + 2 * BETA**2)
    cdf = NormalDist(mu=mu, sigma=sigma).cdf
    mid_point_1 = cdf(-draw_margin)
    mid_point_2 = cdf(draw_margin)
    return 1 - mid_point_2, mid_point_2 - mid_point_1, mid_point_1


def math_quality(p1: trueskill.Rating, p2: trueskill.Rating) -> float:
    """ Calculates relative probability of draw between to players relative to probability of a draw between
    two equally skilled players (when draw_margin approaching 0). So it's always between 0 and 1.
    This is what trueskill match quality returns. For the best match this would be maximized.
    That equals the highest chance for a draw regardless of actual draw chance in given game."""

    sigma2 = 2 * BETA**2 + p1.sigma**2 + p2.sigma**2
    first_part = math.sqrt((2 * BETA**2) / sigma2)
    second_second = math.exp(-1 * ((p1.mu - p2.mu)**2) / (2 * sigma2))
    return first_part * second_second


def main():
    MU_RES = int(MU * 2 + 1)
    SIGMA_RES = int(SIGMA * 3 + 1)
    mu_diffs = np.linspace(-MU_RES, MU_RES, 200)
    sigma_diffs = np.linspace(-SIGMA_RES, SIGMA_RES, 200)
    x, y = np.meshgrid(mu_diffs, sigma_diffs)

    fig = plt.figure(figsize=(10, 8))
    ax = plt.axes(projection='3d')

    cutoff = 0.4
    performance_differences = []

    # Trueskill
    @np.vectorize
    def get_trueskill_quality(mu_diff: float, sigma_diff: float) -> float:
        if sigma_diff >= 0:
            p1 = trueskill.Rating(mu=MU, sigma=SIGMA)
            p2 = trueskill.Rating(mu=MU + mu_diff, sigma=SIGMA + sigma_diff)
        else:
            p1 = trueskill.Rating(mu=MU, sigma=SIGMA - sigma_diff)
            p2 = trueskill.Rating(mu=MU + mu_diff, sigma=SIGMA)

        quality = trueskill.quality_1vs1(p1, p2)

        if quality > cutoff:
            p1w, draw, p2w = match_outcomes(p1, p2)
            performance_differences.append(abs(p1w - p2w))

        return quality

    ts_match_quality = get_trueskill_quality(x, y)
    ax.plot_surface(x, y, ts_match_quality, cmap='viridis', edgecolor='green')

    print("Maximum match quality:", np.max(ts_match_quality))
    print("Maximum perf difference:", max(performance_differences),
          "for cutoff:", cutoff)

    ax.set_title("Trueskill match quality")
    ax.set_xlabel("mu difference")
    ax.set_ylabel("sigma difference")
    ax.set_zlabel("Match quality")
    plt.show()


if __name__ == "__main__":
    main()
