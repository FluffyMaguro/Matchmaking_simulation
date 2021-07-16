import json
import matplotlib.pyplot as plt
from numpy import right_shift
import seaborn as sns
import statistics
import math

with open("data.json", 'r') as f:
    data = json.load(f)

    ### PLOTTING
plt.rcParams['figure.dpi'] = 150


def chance_skill(diff):
    """ Returns the chance for a player to win based on skill difference"""
    return 1 / (1 + math.exp(-10 * (diff)))


## PLAYER HISTORY
def plot_mmr_history(DATAVALUES=6):
    unique_opponents = [len(set(i["opponent_history"])) for i in data]
    extremes = [
        p for p in sorted(data, key=lambda x: x["skill"], reverse=True)
    ]
    players = data[:DATAVALUES - 2] + [extremes[0], extremes[-1]]
    fig, ax = plt.subplots(5, 1)

    for i in range(3):
        ax[0].plot(range(len(extremes[i]['mmr_history'])),
                   extremes[i]['mmr_history'],
                   label=f"Player skill: {extremes[i]['skill']}")
        chances = [
            chance_skill(extremes[i]['skill'] - opponent)
            for opponent in extremes[i]['opponent_history']
        ]

        ax[1].scatter(range(len(chances)), chances, s=2)
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
    chances = [chance_skill(p['skill'] - opp) for opp in p['opponent_history']]
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
    chances = [chance_skill(p['skill'] - opp) for opp in p['opponent_history']]
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
    ax[1].set_ylim(-0.15, 2)
    ax[0].set_title(
        "How player MMR and opponents change\n"
        f"Average unique opponents per player: {statistics.mean(unique_opponents)}"
    )
    ax[0].grid(alpha=0.2)
    ax[1].grid(alpha=0.2)
    fig.set_figheight(8)
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
    plt.savefig("Skill_dist.png")

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

    lines = 0
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

    plt.text(plt.xlim()[1] * 0.93, plt.ylim()[1] * 0.93, f"#{lines}")

    ax2 = ax1.twinx()
    ndata = [i for i in sorted(data, key=lambda x: x["mmr"])]
    nmmrs = [i["mmr"] for i in ndata]
    histories = [len(i["opponent_history"]) for i in ndata]
    ax2.scatter(nmmrs, histories, s=2)
    ax2.set_ylabel(
        f"Game count per player ({min(histories)}-{max(histories)})",
        color='#0b47bf')
    ax2.set_ylim(0, max(histories))
    plt.tight_layout()
    plt.grid(alpha=0.2)
    plt.savefig("MMR_dist.png")

    ## Games played
    plt.figure().clear()
    games_played = [len(i["opponent_history"]) for i in data]
    sns.histplot(games_played, element='poly')
    plt.xlabel("Games played")
    plt.ylabel("Player count")
    plt.title(
        f"Number of games per player\nMedian: {statistics.median(games_played):.0f}"
    )
    plt.grid(alpha=0.2)
    plt.savefig("Games_played.png")


plot_other()