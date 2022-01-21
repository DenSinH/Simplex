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
    circles = lambda x, y: (0.5 * math.cos(x), 0.5 * math.sin(x), 0) if y < 0.5 else (1 + 0.5 * math.cos(x), 0.5 * math.sin(x), 0)
    circle = lambda x: (math.cos(x), math.sin(x), 0)
    # use generate_regular for these:
    sphere = lambda phi, theta: (math.cos(phi) * math.sin(theta), math.sin(phi) * math.sin(theta), math.cos(theta))
    r = 0.3
    R = 1
    torus = lambda phi, theta: (
        (r * math.cos(theta) + R) * math.cos(phi),
        (r * math.cos(theta) + R) * math.sin(phi),
        r * math.sin(theta)
    )

    with open("points.csv", "w+") as f:
        for p in generate_random(sphere, 100, [2 * 3.1416, 3.1416], stddev=0.02):
            f.write(",".join(str(coord) for coord in p) + "\n")
        # for phi in range(50):
        #     for theta in range(10):
        #         f.write(",".join(str(coord) for coord in torus(2 * phi * 3.1416 / 50, 2 * theta * 3.1416 / 10)) + "\n")
