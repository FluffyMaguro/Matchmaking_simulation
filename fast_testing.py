import os
import pickle
from concurrent.futures import ProcessPoolExecutor
from pprint import pprint

import psimulation

CACHE = "parameter_optimization.dat"
PLAYERS = 20000

GAMES = 20000000
REPEATS = 6
STRATEGY = "tweaked2_elo"


def load_data():
    if not os.path.isfile(CACHE):
        return dict()
    with open(CACHE, "rb") as f:
        data = pickle.load(f)
    return data


def save_data(data):
    with open(CACHE, "wb") as f:
        pickle.dump(data, f)


def main():
    pool = ProcessPoolExecutor(10)
    results = []
    data = load_data()
    todo = 0

    # Add work

    for game_div in range(15, 26, 2):
        for KK in range(90, 110, 2):
            for coef in [1]:
                todo += 1

                params = (STRATEGY, 6, KK, game_div, coef)
                if params in data:
                    continue

                for _ in range(REPEATS):
                    results.append(
                        (params,
                         pool.submit(psimulation.run_parameter_optimization,
                                     PLAYERS, GAMES, *params)))

    print(f"Starting at {REPEATS*todo - len(results)}/{REPEATS*todo}\n")

    # Analyze prediction sums
    for idx, (params, future) in enumerate(results):
        if params not in data:
            data[params] = list()

        data[params].append(future.result())
        save_data(data)
        print(f"{idx}/{len(results)}")

    # Average over repeats
    for params in data:
        if len(data[params]) == 2:
            continue
        av1 = sum(i[0] for i in data[params]) / len(data[params])
        av2 = sum(i[1] for i in data[params]) / len(data[params])
        data[params] = (av1, av2)

    save_data(data)

    # Print sorted data
    data = {k: v for k, v in sorted(data.items(), key=lambda x: x[1][0])}
    data = {k: v for k, v in sorted(data.items(), key=lambda x: x[1][1])}
    pprint(data, sort_dicts=False)


if __name__ == "__main__":
    main()
