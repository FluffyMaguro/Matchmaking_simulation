import time

import matplotlib.pyplot as plt
import matplotlib.ticker as mtick
import numpy as np
import seaborn as sns

import psimulation

### RUN SIMULATION
PLAYERS = 20000
GAMES = 5000000

strategy_types = ["naive", "elo", "tweaked_elo", "tweaked2_elo"]
fig, ax = plt.subplots(4, 1, dpi=120, figsize=(7, 18))
BINS = 200
legend = [[], []]
start = time.time()


def smooth(array: np.ndarray, bins: int) -> np.ndarray:
    """ Smooths out a vector by averaging inside bins"""
    L = int(array.size / bins)  # bin length
    new = np.array([])
    for i in range(bins):
        new = np.append(new, np.average(array[L * i:L * (i + 1)]))
    return new


skills = None
for idx, strategy in enumerate(strategy_types):
    data, prediction_differences, match_accuracy = psimulation.run_simulation(
        PLAYERS, GAMES, strategy)

    # MMR - SKILL
    data = np.array([i for i in sorted(data, key=lambda x: x["skill"])])
    if skills is None:
        skills = np.array([i["skill"] for i in data])
    mmrs = np.array([i["mmr"] for i in data])
    ax[3].plot(skills, mmrs)

    # Other plots
    x = np.linspace(1, BINS, BINS)
    legend[1].append(f"{strategy} ({np.sum(match_accuracy)/100000:.2f})")
    legend[0].append(
        f"{strategy} ({np.sum(prediction_differences)/100000:.2f})")

    prediction_differences = smooth(prediction_differences, BINS)
    match_accuracy = smooth(match_accuracy, BINS)

    # Plot & save legend
    p = ax[1].plot(x * GAMES / BINS, match_accuracy)
    color = p[0].get_color()
    ax[0].plot(x * GAMES / BINS, prediction_differences, color=color)

    sns.histplot(match_accuracy,
                 element='poly',
                 fill=True,
                 alpha=0.2,
                 ax=ax[2],
                 color=color)

ax[3].set_ylim(np.min(skills) * 1.1 - 200, np.max(skills) * 1.1 + 200)
ax[3].plot([np.min(skills), np.max(skills)],
           [np.min(skills), np.max(skills)],
           color="black",
           linewidth=0.5)

ax[3].set_title(f"MMR - Skill relation ({GAMES/PLAYERS:.0f} games/player)")
ax[3].set_xlabel("Player skill")
ax[3].set_ylabel("Player MMR")
ax[3].grid(alpha=0.2)
ax[3].legend(strategy_types + ["optimum"], loc="best")

ax[0].set_title(
    f"How a matchmaking strategy gets better at predicting outcomes")
ax[0].set_ylabel("Error in predition")
ax[1].set_title(f"Difference in skill between matched players")
ax[1].set_ylabel("Difference in skill")
ax[2].set_xlabel("Match accuracy error")
ax[2].grid(alpha=0.2)
ax[2].legend(strategy_types, loc="best")

for i in range(2):
    ax[i].grid(alpha=0.2)
    ax[i].set_xlabel("Games")
    ax[i].legend(legend[i], loc="best")
    ax[i].ticklabel_format(style='plain', useOffset=False)

plt.tight_layout()
plt.savefig("img/comparing_strategies.png")
print(f"Everything finished in {time.time()-start:.2f} seconds")
