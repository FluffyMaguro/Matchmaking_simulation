# SIGMA EVOLUTION
import matplotlib.pyplot as plt
import numpy as np


def plot_sigma(data: list):
    fig, ax = plt.subplots(1, 1)
    for player in data:
        L = player['sigma_history'].size
        x = np.arange(1, L + 1)
        plt.plot(x, player['sigma_history'], linewidth=0.3)

    ax.set_ylim(bottom=0)
    ax.set_xlabel("Games")
    ax.set_ylabel("Sigma")
    ax.set_title("How sigma changes (Trueskill)")
    ax.grid(alpha=0.2)
    fig.tight_layout()
    fig.savefig("img/sigma_evolution.png")
