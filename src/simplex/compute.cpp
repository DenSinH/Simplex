#include "compute.h"


Compute::Compute(std::vector<point3d>&& points) : points(std::move(points)) {

}


template<>
std::vector<i32> Compute::FindSimplexDrawIndices<0>([[maybe_unused]] float epsilon) {
    // 0 simplices are always all points, regardless of epsilon
    std::vector<i32> indices = {};
    indices.reserve(points.size());

    for (int i = 0; i < points.size(); i++) {
        indices.push_back(i);
    }
    return indices;
}

template<>
std::vector<i32> Compute::FindSimplexDrawIndices<1>(float epsilon) {
    return FindSimplexDrawIndices<0>(epsilon);
}

template<>
std::vector<i32> Compute::FindSimplexDrawIndices<2>(float epsilon) {
    return FindSimplexDrawIndices<0>(epsilon);
}
