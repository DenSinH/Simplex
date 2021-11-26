#pragma once

#include "point.h"
#include "default.h"

#include <vector>

struct Compute {
    Compute(std::vector<point3d>&& points);

    std::vector<point3d> points;

    template<size_t n>
    std::vector<i32> FindSimplexDrawIndices(float epsilon);
};