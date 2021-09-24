import os
import pickle
from concurrent.futures import ProcessPoolExecutor
from pprint import pprint
import traceback
import psimulation
from mutate import Mutate

CACHE = "parameter_optimization.dat"
PLAYERS = 20000

GAMES = 5000000
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


def old_optimization(REPEATS=1):
    pool = ProcessPoolExecutor(4)
    results = []
    data = load_data()
    todo = 0

    # Add work
    for K in range(2, 9, 2):
        for KK in range(100, 160, 10):
            for game_div in range(70, 160, 20):
                for coef in range(1, 10, 3):

                    todo += 1

                    params = (STRATEGY, K, KK, game_div, coef / 10)
                    if params in data:
                        continue

                    for _ in range(REPEATS):
                        results.append(
                            (params,
                             pool.submit(
                                 psimulation.run_parameter_optimization_nt,
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
        if len(data[params]) == 0:
            continue
        if isinstance(data[params], list):
            av1 = sum(i[0] for i in data[params]) / len(data[params])
            av2 = sum(i[1] for i in data[params]) / len(data[params])
            data[params] = (av1, av2)
        else:
            data[params] = data[params]

    save_data(data)

    # Print sorted data
    data = {k: v for k, v in sorted(data.items(), key=lambda x: x[1][0])}
    data = {k: v for k, v in sorted(data.items(), key=lambda x: x[1][1])}
    pprint(data, sort_dicts=False)


def flatten(data):
    ndata = dict()
    for params in tuple(data):
        if len(data[params]) == 0:
            continue
        if isinstance(data[params], list):
            av1 = sum(i[0] for i in data[params]) / len(data[params])
            av2 = sum(i[1] for i in data[params]) / len(data[params])
            ndata[params] = (av1, av2)
        else:
            ndata[params] = data[params]

    ndata = {
        k: v
        for k, v in sorted(ndata.items(), key=lambda x: x[1][0], reverse=False)
    }
    ndata = {
        k: v
        for k, v in sorted(ndata.items(), key=lambda x: x[1][1], reverse=False)
    }

    return ndata


class Mutator_optimization():
    def __init__(self, REPEATS=1):
        self.pool = ProcessPoolExecutor(4)
        self.results = []
        self.mutator = Mutate((6, 135, 15, 1), top=30)
        self.repeats = REPEATS

        # Load data
        self.data = load_data()

        # Initialize mutator from data
        for params, val in self.data.items():
            if isinstance(val, list):
                for i in val:
                    self.mutator.update_results(params[1:], i[0] * i[1])
            elif isinstance(val, tuple):
                self.mutator.update_results(params[1:], val[0] * val[1])
            else:
                print(params, val)

    def run(self):
        runtimes = 0
        while True:
            # Generate some work
            for _ in range(20):
                params = (STRATEGY, *self.mutator.get_new_parameters())
                for _ in range(self.repeats):
                    self.results.append(
                        (params,
                         self.pool.submit(
                             psimulation.run_parameter_optimization_nt,
                             PLAYERS, GAMES, *params)))

            # Analyze prediction sums
            for idx, (params, future) in enumerate(self.results):
                runtimes += 1
                if params not in self.data:
                    self.data[params] = list()
                try:
                    res = future.result()
                except Exception:
                    print(f"{params=}")
                    traceback.print_exc()
                    continue

                self.data[params].append(res)
                save_data(self.data)

                self.mutator.update_results(params[1:], res[0] * res[1])
                del self.results[idx]

                if runtimes % 30 == 0:
                    self.print_results()

                # Generate new if too few results left
                if len(self.results) < 15:
                    continue

    def print_results(self):
        # Average over repeats
        data = flatten(self.data)
        # Print
        print("\nTOP RESULTS:")
        for idx, (key, value) in enumerate(data.items()):
            print(idx + 1, key, value)
            if idx == 5:
                break
        print("\n====================================")


def retest(TOP=100, REP=12):
    """ Retest `TOP` number of results with `REP` repetitions and updates data with new values"""
    # Average over repeats
    data = flatten(load_data())
    pool = ProcessPoolExecutor(4)
    results = []
    i = 0
    for params in tuple(data)[:TOP]:
        for _ in range(REP):
            results.append(
                (i, params,
                 pool.submit(psimulation.run_parameter_optimization_nt,
                             PLAYERS, GAMES, *params)))
        i += 1

    final = dict()
    for i, params, future in results:
        out = future.result()
        if params not in final:
            final[params] = (i, data[params], out, 1)
        else:
            K = final[params][3]
            av1 = (final[params][2][0] * K + out[0]) / (K + 1)
            av2 = (final[params][2][1] * K + out[1]) / (K + 1)
            final[params] = (i, data[params], (av1, av2), K + 1)

    final = {
        k: v
        for k, v in sorted(
            final.items(), key=lambda x: x[1][2][0], reverse=False)
    }
    final = {
        k: v
        for k, v in sorted(
            final.items(), key=lambda x: x[1][2][1], reverse=False)
    }

    for params in final:
        data[params] = final[params][2]

    save_data(data)
    # pprint(final, width=240, sort_dicts=False)


def main():
    old_optimization(REPEATS=1)

    # M = Mutator_optimization()
    # M.print_results()
    # M.run()

    # retest(TOP=10, REP=3)

    # data = load_data()
    # data = flatten(data)
    # for params in tuple(data)[:20]:
    #     print(params, data[params])


if __name__ == "__main__":
    main()
