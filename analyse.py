import json
import matplotlib.pyplot as plt
import seaborn as sns
import statistics
import math

with open("data.json", 'r') as f:
    data = json.load(f)

    ### PLOTTING
plt.rcParams['figure.dpi'] = 150


## PLAYER HISTORY
def plot_mmr_history(DATAVALUES=6):
    unique_opponents = [len(set(i["opponent_history"])) for i in data]
    extremes = [p for p in sorted(data, key=lambda x: x["skill"])]
    extremes = [extremes[0], extremes[-1]]
    players = data[:DATAVALUES - 2] + extremes

    fig, ax = plt.subplots()

    ax.plot(range(len(extremes[1]['mmr_history'])),
            extremes[1]['mmr_history'],
            label=f"Player skill: {extremes[1]['skill']}")

    ax.set_ylabel("MMR", color="#0b47bf")
    ax.set_xlabel("Games")

    chances = [
        1 / (1 + math.exp(-10 * (extremes[1]['skill'] - opponent)))
        for opponent in extremes[1]['opponent_history']
    ]
    ax2 = ax.twinx()
    ax2.plot(chances, color='red', linewidth=0.5)
    ax2.set_ylim(0, 1.05)
    ax2.set_ylabel("Chances against opponents", color="red")

    plt.title(f"The best player ({extremes[1]['skill']})")
    plt.savefig("Player_extremes.png")

    plt.figure().clear()
    fig, ax = plt.subplots(2, 1)

    for player in players:
        mmr = player["mmr_history"]
        opp = player["opponent_history"]
        p = ax[0].plot(list(range(len(mmr))), mmr, linewidth=0.3)
        ax[0].text(len(mmr) - 0.9,
                   mmr[-1],
                   f'{player["skill"]:.3f}',
                   ha="left",
                   va="center",
                   color=p[0].get_color())
        p = ax[1].plot(list(range(len(opp))), opp, linewidth=0.3)
        ax[1].text(len(opp) + 1,
                   opp[-1],
                   len(set(opp)),
                   ha="left",
                   color=p[0].get_color())

    ax[0].set_ylabel("MMR")
    ax[0].set_xlim(0, ax[0].get_xlim()[1] * 1.1)
    ax[1].set_ylabel("Opponent skill")
    ax[1].set_xlabel("Games")
    ax[1].set_xlim(0, ax[1].get_xlim()[1] * 1.1)
    ax[1].set_ylim(0, 2)
    ax[0].set_title(
        "How player MMR and opponents change\n"
        f"Average unique opponents per player: {statistics.mean(unique_opponents)}"
    )
    plt.savefig("Player_history.png")


plot_mmr_history()

### Sort data
data = [i for i in sorted(data, key=lambda x: x["skill"])]
skills = [i["skill"] for i in data]
mmrs = [i["mmr"] for i in data]


def plot_other():
    ## MMR - SKILL
    plt.figure().clear()
    plt.plot(skills, mmrs)
    plt.title("MMR - Skill relation")
    plt.xlabel("Player skill")
    plt.ylabel("Player MMR")
    plt.grid(alpha=0.2)
    plt.savefig("MMR-Skill.png")

    ## Player skill dist
    plt.figure().clear()
    sns.histplot(skills, element='poly', fill=True, alpha=0.3)
    plt.title("Player skill distribution")
    plt.xlabel("Player skill")
    plt.ylabel("Player count")
    plt.grid(alpha=0.2)
    plt.savefig("Skill_dist.png")

    # Player MMR dist
    plt.figure().clear()
    fig, ax1 = plt.subplots()

    sns.histplot(mmrs, element='poly', color='red', fill=True, alpha=0.3)
    plt.title("Player MMR distribution")
    plt.xlabel("Player MMR")
    ax1.set_ylabel("Player count", color='red')
    plt.grid(alpha=0.2)

    ax2 = ax1.twinx()
    ndata = [i for i in sorted(data, key=lambda x: x["mmr"])]
    nmmrs = [i["mmr"] for i in ndata]
    histories = [len(i["opponent_history"]) for i in ndata]
    ax2.scatter(nmmrs, histories, s=2)
    ax2.set_ylabel(
        f"Game count per player ({min(histories)}-{max(histories)})",
        color='#0b47bf')
    plt.tight_layout()
    plt.savefig("MMR_dist.png")

    # Games played
    plt.figure().clear()
    games_played = [len(i["opponent_history"]) for i in data]
    sns.histplot(games_played, element='poly')
    plt.xlabel("Games played")
    plt.ylabel("Player count")
    plt.title(
        f"Number of games per player\nMedian: {statistics.median(games_played):.0f}"
    )
    plt.savefig("Games_played.png")


plot_other()