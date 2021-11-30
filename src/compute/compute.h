#pragma once

#include "point.h"
#include "simplex.h"
#include "column.h"
#include "default.h"

#include <vector>
#include <boost/unordered_set.hpp>
#include <boost/unordered_map.hpp>
#include <boost/container/static_vector.hpp>


template<size_t N>
struct Compute {
    using simplex_t = Simplex<N>;
    using column_t = Column<N>;
    using basis_t = std::vector<column_t>;
    using point_t = point_max;

    Compute(const std::vector<point_t>& points) : points(points) {

    }

    const std::vector<point_t>& points;

    int current_simplices;

    float last_epsilon = -1;
    std::array<std::array<bool, N>, N> one_simplex_cache{};
    std::vector<boost::unordered_set<simplex_t>> simplex_cache{};

    void ClearSimplexCache(float epsilon);

    template<size_t n, class F>
    void ForEachSimplex(float epsilon, const F& func);
    boost::container::static_vector<std::vector<i32>, 3> FindSimplexDrawIndices(float epsilon, int n);
    template<int n>
    std::pair<basis_t, basis_t> FindBZn(float epsilon);

private:
    template<size_t n>
    std::vector<i32> FindSimplexDrawIndicesImpl(float epsilon);
    std::pair<basis_t, basis_t> FindBZ0(float epsilon);
    void Find1Simplices(float epsilon);

    float Distance2(int i, int j) const {
        float dist = 0;
        for (int c = 0; c < point_t::dim; c++) {
            const float dx = points[i][c] - points[j][c];
            dist += dx * dx;
        }
        return dist;
    }
};


template<size_t N>
template<size_t n, class F>
void Compute<N>::ForEachSimplex(float epsilon, const F& func) {
    // 0 simplices are always just the points
    if constexpr(n == 0) {
        for (int i = 0; i < points.size(); i++) {
            func(simplex_t{i});
        }
        return;
    }

    // clear cache if needed
    ClearSimplexCache(epsilon);

    if (simplex_cache.size() < n) {
        simplex_cache.resize(n);
    }

    // 1 simplices are special since we can use them for the higher order simplices
    if constexpr(n == 1) {
        Find1Simplices(epsilon);
    }

    // use cached simplices
    if (!simplex_cache[n - 1].empty()) {
        for (simplex_t s : simplex_cache[n - 1]) {
            func(s);
        }
        return;
    }

    if constexpr(n > 1) {
        auto& simplices = simplex_cache[n - 1] = {};
        // first find all 1-simplices
        Find1Simplices(epsilon);
        ForEachSimplex<n - 1>(epsilon, [&](simplex_t s) {
            // try every other point
            for (int i = 0; i < points.size(); i++) {
                if (s[i]) [[unlikely]] {
                    // point is already contained in the simplex
                    continue;
                }
                auto next = s | simplex_t{i};
                if (simplices.find(next) != simplices.end()) {
                    // simplex is already found
                    continue;
                }

                if (!s.ForEachPoint([&](int p) -> bool {
                    // check whether there is a 1-simplex for every point in the simplex
                    if (!one_simplex_cache[i][p]) {
                        return true;  // bad simplex, 1-simplex does not exist
                    }
                    return false;  // keep going, 1-simplex exists for this point
                })) {
                    simplices.insert(next);
                    func(next);
                }
            }
        });
    }
}