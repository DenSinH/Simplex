import random


def generate_random(parameterization: callable, num: int, bounds, stddev: float = 0.1):
    for i in range(num):
        point = parameterization(*[random.uniform(0, bound) for bound in bounds])
        yield tuple(coord + random.normalvariate(0, stddev) for coord in point)


def generate_regular(parameterization: callable, num: int, bounds, stddev: float = 0.1):
    def n_range(dim, n):
        if dim == 1:
            for i in range(n):
                yield (i,)
        else:
            for i in range(n):
                for rest in n_range(dim - 1, n):
                    yield i, *rest

    params = len(bounds)
    points_per_dim = int(num ** (1/float(params)))
    for fracs in n_range(params, points_per_dim):
        point = parameterization(*[frac * bound / points_per_dim for frac, bound in zip(fracs, bounds)])
        yield tuple(coord + random.normalvariate(0, stddev) for coord in point)


if __name__ == '__main__':
    import math
    # circles = lambda x, y: (0.5 * math.cos(x), 0.5 * math.sin(x), 0) if y < 0.5 else (1 + 0.5 * math.cos(x), 0.5 * math.sin(x), 0)
    # circle = lambda x: (math.cos(x), math.sin(x), 0)
    sphere = lambda phi, theta: (math.cos(phi) * math.sin(theta), math.sin(phi) * math.sin(theta), math.cos(theta))

    with open("points.csv", "w+") as f:
        for p in generate_regular(sphere, 25, [2 * 3.1416, 3.1416], stddev=0.02):
            f.write(",".join(str(coord) for coord in p) + "\n")
