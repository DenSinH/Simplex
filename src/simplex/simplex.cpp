#include "simplex.h"

#include <thread>


Simplex::Simplex(std::vector<point3d>&& points) : points(std::move(points)) {

}


template<>
std::vector<i32> Simplex::FindSimplexDrawIndices<0>([[maybe_unused]] float epsilon) {
    std::vector<i32> indices = {};
    indices.reserve(points.size());

    for (int i = 0; i < points.size(); i++) {
        indices.push_back(i);
    }
    return indices;
}

template<>
std::vector<i32> Simplex::FindSimplexDrawIndices<1>(float epsilon) {
    return FindSimplexDrawIndices<0>(epsilon);
}

template<>
std::vector<i32> Simplex::FindSimplexDrawIndices<2>(float epsilon) {
    return FindSimplexDrawIndices<0>(epsilon);
}
