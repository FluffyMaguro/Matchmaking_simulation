import os
import pickle
from concurrent.futures import ProcessPoolExecutor
from pprint import pprint

import psimulation

PLAYERS = 20000
GAMES = 20000000
REPEATS = 6
CACHE = "parameter_optimization.dat"


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

    # Add work
    for K in [6, 7]:
        for KK in range(100, 150, 5):
            for game_div in range(5, 50, 5):

                params = (K, KK, game_div)
                if params in data:
                    continue

                for _ in range(REPEATS):
                    results.append(
                        (params,
                         pool.submit(psimulation.run_parameter_optimization,
                                     PLAYERS, GAMES, *params)))

    print(f"Starting at {len(data)}/{len(results) + len(data)}\n")

    # Analyze prediction sums
    for params, future in results:
        if params not in data:
            data[params] = list()

        data[params].append(future.result())
        save_data(data)

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
