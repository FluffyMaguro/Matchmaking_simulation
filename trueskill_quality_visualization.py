import math

import matplotlib.colors as colors
import matplotlib.pyplot as plt
import numpy as np
import trueskill

from trueskill_helpers import (MU, SIGMA, match_outcomes, match_quality,
                               visualize_match_outcomes)


def main():
    RES = 100
    MU_RES = int(MU * 2 + 1)
    SIGMA_RES = int(SIGMA * 4 + 1)
    mu_diffs = np.linspace(-MU_RES, MU_RES, RES)
    sigmas = np.linspace(0.0001, SIGMA_RES, RES)
    x, y = np.meshgrid(mu_diffs, sigmas)

    fig = plt.figure(figsize=(10, 8))
    ax = plt.axes(projection='3d')

    CUTOFF = 0.4
    winrates = []

    # Trueskill
    @np.vectorize
    def get_trueskill_quality(mu_diff: float, sigma: float) -> float:
        small_sigma = sigma = sigma / math.sqrt(
            2)  # So both lead to total of sigma
        p1 = trueskill.Rating(mu=MU, sigma=small_sigma)
        p2 = trueskill.Rating(mu=MU + mu_diff, sigma=small_sigma)
        quality = trueskill.quality_1vs1(p1, p2)

        if quality > CUTOFF:
            p1w = match_outcomes(p1, p2)[0]
            winrates.append(p1w)

        return quality

    @np.vectorize
    def get_close_to_50(mu_diff: float, sigma: float) -> float:
        small_sigma = sigma / math.sqrt(2)  # So both lead to total of sigma
        p1 = trueskill.Rating(mu=MU, sigma=small_sigma)
        p2 = trueskill.Rating(mu=MU + mu_diff, sigma=small_sigma)
        p1w = match_outcomes(p1, p2)[0]
        return 1.5 - abs(0.5 - p1w) # offset for visibility

    # custom color map denoting the CUTOFF
    my_colors = [(0, 0, 0.5), (0, 0, 1), (0.2, 0.8, 0.2), (0, 0.8, 0)]
    my_cmap = colors.LinearSegmentedColormap.from_list('my_cmap',
                                                       my_colors,
                                                       N=4)
    divnorm = colors.TwoSlopeNorm(vmin=0, vcenter=CUTOFF, vmax=1)

    ts_match_quality = get_trueskill_quality(x, y)
    ax.plot_surface(
        x,
        y,
        ts_match_quality,
        cmap=my_cmap,
        norm=divnorm,
        edgecolor="black",
        linewidth=0.1,
    )

    win_predictions = get_close_to_50(x, y)
    ax.plot_surface(x, y, win_predictions, cmap='inferno')

    print(f"Maximum match quality: {np.max(ts_match_quality):.3f}")
    print(f"Maximum winrate: {max(winrates):.2%} for cutoff: {CUTOFF}")

    ax.set_title("Trueskill match quality")
    ax.set_xlabel("mu difference")
    ax.set_ylabel(f"Total sigma (default={math.sqrt(2)*SIGMA:.2f})")
    ax.set_zlabel("Match quality")
    plt.tight_layout()
    plt.show()


if __name__ == "__main__":
    main()
