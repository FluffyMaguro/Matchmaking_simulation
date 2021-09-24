import random
import traceback
from typing import Tuple


class Mutate:
    """ A class for generating paramaters based on previous one success
    
    `diff` is how much a parameter changes per mutation
    `top` is how many top candidates are considered
    """
    def __init__(self, params: Tuple, diff: float = 1, top: int = 5):
        self.init_params = params
        self.results = {}

        self.diff = diff
        self.top = top

    def update_results(self, params: Tuple, result: float):
        """ Accepts some metric result for params
        
        Lower result the better"""
        if self.results.get(params) is None:
            self.results[params] = (result, 1)  # Save the number of updates
        else:
            # Average with previous results if there are some
            K = self.results[params][1]
            try:
                average = (self.results[params][0] * K + result) / (K + 1)
                self.results[params] = (average, K + 1)
            except Exception:
                traceback.print_exc()
                print(f"{params=} {result=}")

    @staticmethod
    def sorting_function(value):
        if value is None or value[1] is None:
            return 9999999999
        else:
            return value[1][0]

    def sort_results(self):
        """ Sort results by metric (lowest first)"""
        self.results = {
            k: v
            for k, v in sorted(self.results.items(), key=self.sorting_function)
        }

    def get_new_parameters(self) -> Tuple:
        """ Gets new parameters by modifying old ones"""

        self.sort_results()

        while True:
            # Choose which parameters to mutate
            idx = random.randint(0, min(self.top, len(self.results)) - 1)
            params = list(list(self.results.keys())[idx])
            best = params.copy()

            # And which one exactly
            idx = random.randint(0, len(params) - 1)
            diff = self.diff

            # Special case for tweaked2
            if idx == 3:
                diff /= 5

            # Choose whether to increase or decrease
            if random.getrandbits(1):
                params[idx] += diff
            else:
                params[idx] -= diff

            # Check for too low values
            if params[idx] < 1:
                continue

            params = tuple(params)
            if params not in self.results:
                print(f"Mutating {best} → {params}")
                return params
