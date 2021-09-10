import time

import matplotlib.pyplot as plt
import matplotlib.ticker as mtick
import numpy as np
import seaborn as sns

import psimulation

### RUN SIMULATION
PLAYERS = 20000
GAMES = 200000

strategy_types = ["naive", "elo", "tweaked_elo", "tweaked2_elo"]
fig, ax = plt.subplots(3, 1, dpi=120, figsize=(7, 11))
BINS = 200
legend = [[], []]
start = time.time()


def smooth(array: np.array, bins: int) -> np.array:
    """ Smooths out a vector by averaging inside bins"""
    L = int(array.size / bins)  # bin length
    new = np.array([])
    for i in range(bins):
        new = np.append(new, np.average(array[L * i:L * (i + 1)]))
    return new


for idx, strategy in enumerate(strategy_types):
    _, prediction_differences, match_accuracy = psimulation.run_simulation(
        PLAYERS, GAMES, strategy)

    legend[1].append(f"{strategy} ({np.sum(match_accuracy)/100000:.2f})")
    legend[0].append(
        f"{strategy} ({np.sum(prediction_differences)/100000:.2f})")

    x = np.linspace(1, BINS, BINS)
    prediction_differences = smooth(prediction_differences, BINS)
    match_accuracy = smooth(match_accuracy, BINS)

    # Plot & save legend
    p = ax[1].plot(x * GAMES / BINS, match_accuracy)
    color = p[0].get_color()
    ax[0].plot(x * GAMES / BINS, prediction_differences, color=color)

    sns.histplot(match_accuracy, element='poly', fill=True, alpha=0.2, ax=ax[2], color=color)

ax[0].set_title(
    f"How a matchmaking strategy gets better at predicting outcomes")
ax[0].set_ylabel("Error in predition")
ax[1].set_title(f"Difference in skill between matched players")
ax[1].set_ylabel("Difference in skill")
ax[2].set_xlabel("Matchmaking error")
ax[2].grid(alpha=0.2)
ax[2].legend(legend[1], loc="best")

for i in range(2):
    ax[i].grid(alpha=0.2)
    ax[i].set_xlabel("Games")
    ax[i].legend(legend[i], loc="best")

plt.tight_layout()
plt.savefig("img/comparing_strategies.png")
print(f"Everything finished in {time.time()-start:.2f} seconds")
