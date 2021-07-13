import json
import matplotlib.pyplot as plt
import seaborn as sns

with open("data.json", 'r') as f:
    data = json.load(f)

    ### PLOTTING
plt.rcParams['figure.dpi'] = 150

## PLAYER HISTORY
def plot_mmr_history(DATAVALUES=6):
    fig, ax = plt.subplots(2, 1)

    for player in data[:DATAVALUES]:
        mmr = player["mmr_history"]
        opp = player["opponent_history"]
        p = ax[0].plot(list(range(len(mmr))), mmr, linewidth=0.3)
        ax[0].text(len(mmr) - 0.9,
                   mmr[-1],
                   f'{player["skill"]:.3f}',
                   ha="left",
                   va="center",
                   color=p[0].get_color())
        ax[1].plot(list(range(len(opp))), opp, linewidth=0.3)

    ax[0].set_ylabel("MMR")
    ax[0].set_xlim(0, ax[0].get_xlim()[1] * 1.1)
    ax[1].set_ylabel("Opponent skill")
    ax[1].set_xlabel("Games")
    ax[1].set_ylim(0, 2)
    ax[0].set_title("How player MMR and opponents change")
    plt.savefig("Player_history.png")


plot_mmr_history(5)

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
    sns.histplot(mmrs, element='poly', color='red', fill=True, alpha=0.3)
    plt.title("Player MMR distribution")
    plt.xlabel("Player MMR")
    plt.ylabel("Player count")
    plt.grid(alpha=0.2)
    plt.savefig("MMR_dist.png")


plot_other()