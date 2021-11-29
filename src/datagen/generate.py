from inspect import signature
import random


def generate(parameterization: callable, num: int, bound: float = 100, stddev: float = 0.1):
    params = len(signature(parameterization).parameters)
    for i in range(num):
        point = parameterization(*[random.uniform(-bound, bound)])
        yield tuple(coord + random.normalvariate(0, stddev) for coord in point)


if __name__ == '__main__':
    import math
    circle = lambda x: (math.cos(x), math.sin(x), 0)

    with open("points.csv", "w+") as f:
        for p in generate(circle, 200, bound=3.2):
            f.write(",".join(str(coord) for coord in p) + "\n")
