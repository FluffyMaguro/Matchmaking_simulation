import math
import os
import statistics
import time
from functools import lru_cache, partial

import matplotlib.pyplot as plt
import numpy as np
import psutil
import seaborn as sns

import psimulation
import trueskill_rate
from plot_sigma import plot_sigma

start = time.time()

### RUN SIMULATION
PLAYERS = 20000
GAMES = 10000000
STRATEGY = "trueskill"
"""
PLAYERS = 20000
GAMES = 100000000
-----------------

BASE
14273 MB    81.82s
14275 MB    82.23s

RESERVED VECTOR SIZE
- no change

COPY
11993 MB                with changing to pointers and carray from std::vector
11987 MB                when simulation releasing its vector pointers
7966 MB     89.1423s    when Players vectors directly to carray as well

"""


@lru_cache()
def chance_skill(diff):
    """ Returns the chance for a player to win based on skill difference"""
    return 1 / (1 + math.exp(-diff / 173.718))  # ELO points


@lru_cache()
def chance_skill_two(a, b):
    """ Returns the chance for a player to win based on skill difference"""
    return chance_skill(a - b)


def plot_data(data, prediction_differences, match_accuracy, PLAYERS, GAMES,
              STRATEGY):

    ### PLOTTING
    start_plotting = time.time()
    plt.rcParams['figure.dpi'] = 150

    plot_sigma(data)

    ## PREDICTION DIFFERENCES
    def plot_prediction_differences():
        plt.figure().clear()
        factor = 50
        L = int(prediction_differences.size / factor)
        means = []
        stdevs = [[], []]
        x = []
        for i in range(factor):
            mean = np.average(prediction_differences[L * i:L * (i + 1)])
            stdev = np.std(prediction_differences[L * i:L * (i + 1)])
            means.append(mean)
            stdevs[0].append(mean - stdev)
            stdevs[1].append(mean + stdev)
            x.append(L * i)

        fig, ax = plt.subplots(1, 1)
        ax.text(0,
                means[0] + 0.01,
                f"{means[0]:.3f}",
                color="#1968cf",
                ha="center")
        ax.text(x[-1],
                means[-1] + 0.01,
                f"{means[-1]:.3f}",
                color="#1968cf",
                ha="center")

        ax.plot(x, means)
        ax.fill_between(x, stdevs[0], stdevs[1], color="#1968cf", alpha=0.2)
        ax.set_ylabel("Error in matchmaking")
        ax.set_xlabel("Games")
        plt.grid(alpha=0.2)
        ax.set_title(
            f"How matchmaking improves [{np.sum(prediction_differences)/100000:.3f}]"
        )
        plt.tight_layout()
        plt.savefig("img/prediction_differences.png")

    start_pred = time.time()
    plot_prediction_differences()
    print(
        f"Prediction plotting finished in {time.time()-start_pred:.3f} seconds"
    )

    ## PLAYER HISTORY
    def plot_mmr_history(DATAVALUES=6):
        plt.figure().clear()
        unique_opponents = [
            len(np.unique(i["opponent_history"])) for i in data
        ]
        extremes = [
            p for p in sorted(data, key=lambda x: x["skill"], reverse=True)
        ]
        players = data[:DATAVALUES - 2] + [extremes[0], extremes[-1]]
        fig, ax = plt.subplots(5, 1)

        for i in range(3):
            ax[0].plot(np.linspace(0, extremes[i]['mmr_history'].size - 1,
                                   extremes[i]['mmr_history'].size),
                       extremes[i]['mmr_history'],
                       label=f"Player skill: {extremes[i]['skill']:.2f}")

            player_chances = np.vectorize(
                partial(chance_skill_two, extremes[i]['skill']))

            chances = player_chances(extremes[i]['opponent_history'])

            ax[1].scatter(np.linspace(0, chances.size - 1, chances.size),
                          chances,
                          s=2)
            if i == 0:
                ax[2].plot(chances, label="True chances")

        ax[0].set_ylabel("MMR")
        ax[0].set_xlabel("Games")
        ax[0].legend()
        ax[0].set_title(f"Top 3 players")
        ax[0].grid(alpha=0.2)
        ax[1].set_ylabel("Chances against opponents")
        ax[1].set_ylim(0, 1.05)
        ax[1].grid(alpha=0.2)
        ax[1].set_xlabel("Games")

        # Plot the best player
        p = extremes[0]
        ax[2].plot(p["predicted_chances"], label="Predicted chances")
        ax[2].legend()
        ax[2].grid(alpha=0.2)
        ax[2].set_title(
            f'Best player ({chance_skill(extremes[0]["skill"]-extremes[1]["skill"]):.2f} chance against second best)'
        )
        ax[2].set_xlabel("Games")
        ax[2].set_ylabel("Changes against the oppponent")
        ax[2].set_ylim(-0.02, 1.02)

        # the worst player
        p = extremes[-1]
        player_chances = np.vectorize(partial(chance_skill_two, p['skill']))
        chances = player_chances(p['opponent_history'])
        ax[4].plot(chances, label="True chances")
        ax[4].plot(p["predicted_chances"], label="Predicted chances")
        ax[4].legend()
        ax[4].grid(alpha=0.2)
        ax[4].set_title(
            f'Worst player ({chance_skill(extremes[-1]["skill"]-extremes[-2]["skill"]):.2f} chance against second worst)'
        )
        ax[4].set_xlabel("Games")
        ax[4].set_ylabel("Changes against the oppponent")
        ax[4].set_ylim(-0.02, 1.02)

        # average player
        p = extremes[int(len(extremes) / 2)]
        player_chances = np.vectorize(partial(chance_skill_two, p['skill']))
        chances = player_chances(p['opponent_history'])
        ax[3].plot(chances, label="True chances")
        ax[3].plot(p["predicted_chances"], label="Predicted chances")
        ax[3].legend()
        ax[3].grid(alpha=0.2)
        ax[3].set_title("Average player")
        ax[3].set_xlabel("Games")
        ax[3].set_ylabel("Changes against the oppponent")
        ax[3].set_ylim(top=1)
        ax[3].set_ylim(-0.02, 1.02)

        fig.set_figheight(16)
        fig.tight_layout(h_pad=2.1)
        plt.savefig("img/Player_extremes.png")

        plt.figure().clear()
        fig, ax = plt.subplots(2, 1)

        for player in players:
            mmr = player["mmr_history"]
            opp = player["opponent_history"]
            p = ax[0].plot(np.linspace(0, mmr.size - 1, mmr.size),
                           mmr,
                           linewidth=0.3)
            ax[0].text(len(mmr) - 0.9,
                       mmr[-1],
                       f'{player["skill"]:.3f}',
                       ha="left",
                       va="center",
                       color=p[0].get_color())
            p = ax[1].plot(np.linspace(0, opp.size - 1, opp.size),
                           opp,
                           linewidth=0.3)
            ax[1].text(opp.size + 1,
                       opp[-1],
                       opp.size,
                       ha="left",
                       color=p[0].get_color())

        ax[0].set_ylabel("MMR")
        ax[0].set_xlim(0, ax[0].get_xlim()[1] * 1.1)
        ax[1].set_ylabel("Opponent skill")
        ax[1].set_xlabel("Games")
        ax[1].set_xlim(0, ax[1].get_xlim()[1] * 1.1)
        ax[0].set_title(
            "How player MMR and opponents change\n"
            f"Average unique opponents per player: {statistics.mean(unique_opponents):.2f}"
        )
        ax[0].grid(alpha=0.2)
        ax[1].grid(alpha=0.2)
        fig.set_figheight(8)
        plt.savefig("img/Player_history.png")

    start_hist = time.time()
    plot_mmr_history()
    print(
        f"MMR history plotting finished in {time.time()-start_hist:.3f} seconds"
    )

    ### Sort data
    data = [i for i in sorted(data, key=lambda x: x["skill"])]
    skills = np.array([i["skill"] for i in data])
    mmrs = np.array([i["mmr"] for i in data])

    def plot_other():
        ## MMR - SKILL
        plt.figure().clear()
        fig, ax = plt.subplots()
        ax.plot(skills, mmrs)

        if STRATEGY == 'trueskill':
            ax2 = ax.twinx()
            ax.set_ylabel("Player MMR", color="#1F4B73")
            ax2.set_ylabel("Player Skill")
        else:
            ax2 = ax
            ax.set_ylabel("Player MMR")

        ax2.plot([np.min(skills), np.max(skills)],
                 [np.min(skills), np.max(skills)],
                 color="black",
                 linewidth=0.5)

        plt.title(f"MMR - Skill relation ({GAMES/PLAYERS:.0f} games/player)")
        plt.xlabel("Player skill")

        plt.grid(alpha=0.2)
        plt.tight_layout()
        plt.savefig("img/MMR-Skill.png")

        ## Player skill dist
        plt.figure().clear()
        sns.histplot(skills, element='poly', fill=True, alpha=0.3)
        plt.title(
            "Player skill distribution\n(lines show where players have 75% chance to win against previous line)"
        )
        plt.xlabel("Player skill")
        plt.ylabel("Player count")
        M = plt.ylim()[1]

        # Plotting lines where a player has 75% chance to win against previous line
        # More lines means more diverse population of skills
        lines = 0
        previous_skill = None
        for skill in skills:
            if previous_skill is None:
                previous_skill = skill
                continue

            chance = chance_skill(skill - previous_skill)
            if chance > 0.75:
                previous_skill = skill
                plt.plot([skill, skill], [0, M], "k--", linewidth=0.5)
                lines += 1

        plt.text(plt.xlim()[1] * 0.92, plt.ylim()[1] * 0.93, f"#{lines}")
        plt.grid(alpha=0.2)
        plt.savefig("img/Skill_dist.png")

        ## Player MMR dist
        plt.figure().clear()
        fig, ax1 = plt.subplots()

        sns.histplot(mmrs, element='poly', color='red', fill=True, alpha=0.3)
        plt.title(
            "Player MMR distribution\n(lines show where players have 75% chance to win against previous line)"
        )
        plt.xlabel("Player MMR")
        ax1.set_ylabel("Player count", color='red')
        M = plt.ylim()[1]

        # Plot lines
        lines = 0
        if STRATEGY != "trueskill":
            previous_mmr = None
            for mmr in mmrs:
                if previous_mmr is None:
                    previous_mmr = mmr
                    continue

                chance = 1 / (1 + math.exp((previous_mmr - mmr) / 173.718))
                if chance > 0.75:
                    previous_mmr = mmr
                    plt.plot([mmr, mmr], [0, M], "k--", linewidth=0.5)
                    lines += 1

        else:
            minimum_mmr = min(mmrs)
            maximum_mmr = max(mmrs)
            current_mmr = minimum_mmr
            while True:
                current_mmr += 4.16666666  # BETA
                if current_mmr > maximum_mmr:
                    break
                plt.plot([current_mmr, current_mmr], [0, M],
                         "k--",
                         linewidth=0.5)
                lines += 1

        plt.text(plt.xlim()[1] * 0.93, plt.ylim()[1] * 0.93, f"#{lines}")
        ax2 = ax1.twinx()
        ndata = [i for i in sorted(data, key=lambda x: x["mmr"])]
        nmmrs = [i["mmr"] for i in ndata]
        histories = [i["opponent_history"].size for i in ndata]
        ax2.scatter(nmmrs, histories, s=2)
        ax2.set_ylabel(
            f"Game count per player ({min(histories)}-{max(histories)})",
            color='#0b47bf')
        ax2.set_ylim(0, max(histories))
        plt.tight_layout()
        plt.grid(alpha=0.2)
        plt.savefig("img/MMR_dist.png")

        ## Games played
        plt.figure().clear()
        games_played = [i["opponent_history"].size for i in data]
        sns.histplot(games_played, element='poly')
        plt.xlabel("Games played")
        plt.ylabel("Player count")
        plt.title(
            f"Number of games per player\nMedian: {statistics.median(games_played):.0f}"
        )
        plt.grid(alpha=0.2)
        plt.savefig("img/Games_played.png")

    plot_other()
    print(
        f"Other plotting finished in {time.time()-start_plotting:.3f} seconds")
    print(f"Total time: {time.time()-start:.3f} seconds")


if __name__ == "__main__":
    psimulation.set_my_python_function(trueskill_rate.rate_1v1)
    data, prediction_differences, match_accuracy, good_match_fraction = psimulation.run_simulation(
        PLAYERS, GAMES, STRATEGY)
    process = psutil.Process(os.getpid())
    print(
        f"Peak memory usage during simulation: {process.memory_info().peak_wset/(1024*1024):.0f} MB"
    )

    plot_data(data, prediction_differences, match_accuracy, PLAYERS, GAMES,
              STRATEGY)