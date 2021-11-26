#pragma once

#include "point.h"
#include "default.h"

#include <vector>

struct Simplex {
    Simplex(std::vector<point3d>&& points);

    std::vector<point3d> points;

    template<size_t n>
    std::vector<i32> FindSimplexDrawIndices(float epsilon);
};