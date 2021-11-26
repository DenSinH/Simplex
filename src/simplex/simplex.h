#pragma once

#include "point.h"

#include <vector>

struct Simplex {
    Simplex(std::vector<point3d>&& points);

    std::vector<point3d> points;
};