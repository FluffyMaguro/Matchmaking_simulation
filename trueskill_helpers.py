import math
from statistics import NormalDist
from typing import Tuple

import matplotlib.pyplot as plt
import numpy as np
import trueskill

MU = 25.  #   25
SIGMA = 25 / 3  #  ~ 8.333
BETA = SIGMA / 2  #  ~ 4.166    # Variance of performance


def match_quality(p1: trueskill.Rating, p2: trueskill.Rating) -> float:
    """ Calculates relative probability of draw between to players relative to probability of a draw between
    two equally skilled players (when draw_margin approaching 0). So it's always between 0 and 1.
    This is what trueskill match quality returns. For the best match this would be maximized.
    That equals the highest chance for a draw regardless of actual draw chance in given game."""

    sigma2 = 2 * BETA**2 + p1.sigma**2 + p2.sigma**2
    first_part = math.sqrt((2 * BETA**2) / sigma2)
    second_second = math.exp(-1 * ((p1.mu - p2.mu)**2) / (2 * sigma2))
    return first_part * second_second


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


def visualize_match_outcomes(p1: trueskill.Rating,
                             p2: trueskill.Rating,
                             draw_margin: int = 0,
                             save_file: str = None,
                             show_rank: bool = True) -> None:
    """ Visualizes a match between two players

    args:
        p1: player 01
        p2: player 02
        draw_margin: how many points in perf difference would lead to a draw
        save_file: if a filename is given it saves the image instead of showing
        show_rank: Shows player rank (mu - 3*sigma)
    
    """
    fig, ax = plt.subplots(2, 1, figsize=(6, 6), dpi=120)

    # Plot players
    COEF = 4
    minimum = min(p1.mu - COEF * p1.sigma, p2.mu - COEF * p2.sigma)
    maximum = max(p1.mu + COEF * p1.sigma, p2.mu + COEF * p2.sigma)
    diff = maximum - minimum
    x = np.linspace(minimum - 0.1 * diff, maximum + 0.1 * diff, 400)

    p1f = np.vectorize(NormalDist(mu=p1.mu, sigma=p1.sigma).pdf)
    p2f = np.vectorize(NormalDist(mu=p2.mu, sigma=p2.sigma).pdf)
    p1plot = ax[0].plot(x, p1f(x), label=f"P1 ({p1.mu:.2f}±{p1.sigma:.2f})")
    p2plot = ax[0].plot(x, p2f(x), label=f"P2 ({p2.mu:.2f}±{p2.sigma:.2f})")

    # Ranks
    if show_rank:
        p1rank = p1.mu - 3 * p1.sigma
        p2rank = p2.mu - 3 * p2.sigma
        ylim = ax[0].get_ylim()[1]
        ax[0].plot([p1rank, p1rank], [ylim * 0.02, ylim * 0.05],
                   color=p1plot[0].get_color())
        ax[0].plot([p2rank, p2rank], [ylim * 0.02, ylim * 0.05],
                   color=p2plot[0].get_color())
        ax[0].text(p1rank,
                   ylim * 0.07,
                   f"rank={p1rank:.0f}",
                   color=p1plot[0].get_color(),
                   horizontalalignment="center")
        ax[0].text(p2rank,
                   ylim * 0.07,
                   f"rank={p2rank:.0f}",
                   color=p2plot[0].get_color(),
                   horizontalalignment="center")

    ax[0].set_xlabel("Performance")
    ax[0].set_ylabel("Probability of performance")
    ax[0].set_title(
        f"Possible outcomes of one match\n(match quality: {match_quality(p1,p2):.3f})"
    )
    ax[0].legend()
    ax[0].set_yticklabels([])
    ax[0].grid(alpha=0.2)

    # Plot chance of winning
    if p1.mu > p2.mu:
        mu = p1.mu - p2.mu
    else:
        mu = p2.mu - p1.mu

    sigma = (p1.sigma**2 + p2.sigma**2)**0.5
    pdf = np.vectorize(NormalDist(mu=mu, sigma=sigma).pdf)
    cdf = np.vectorize(NormalDist(mu=mu, sigma=sigma).cdf)
    x = np.linspace(mu - 3 * sigma, mu + 3 * sigma, 200)
    y = pdf(x)
    ax[1].plot(x, y, color="black", alpha=0.5)
    ax[1].plot([0, 0], [0, max(y)],
               '--',
               color="black",
               alpha=0.5,
               linewidth=0.9)

    # Fill P1
    # Switch order so it nicely corresponds to the player order above
    if p1.mu > p2.mu:
        x = np.linspace(draw_margin, mu + 3 * sigma)
        chance_of_winning = 1 - cdf(draw_margin)
    else:
        x = np.linspace(mu - 3 * sigma, -draw_margin)
        chance_of_winning = cdf(-draw_margin)

    ax[1].fill_between(x,
                       pdf(x),
                       alpha=0.5,
                       label=f"{chance_of_winning:.2%} P1 winning")
    # Fill P2
    if p1.mu > p2.mu:
        x = np.linspace(mu - 3 * sigma, -draw_margin)
        chance_of_winning = cdf(-draw_margin)
    else:
        x = np.linspace(draw_margin, mu + 3 * sigma)
        chance_of_winning = cdf(draw_margin)

    ax[1].fill_between(x,
                       pdf(x),
                       alpha=0.5,
                       label=f"{chance_of_winning:.2%} P2 winning")
    # Fill draw
    if draw_margin:
        x = np.linspace(-draw_margin, draw_margin)
        chance_of_draw = cdf(draw_margin) - cdf(-draw_margin)
        ax[1].fill_between(x,
                           pdf(x),
                           alpha=0.5,
                           label=f"{chance_of_draw:.2%} Draw")

    ax[1].grid(alpha=0.2)
    ax[1].set_xlabel("Performance difference")
    ax[1].set_ylabel("Probability of difference")
    ax[1].set_yticklabels([])
    ax[1].legend()
    plt.tight_layout()

    if save_file:
        plt.savefig(save_file)
    else:
        plt.show()
