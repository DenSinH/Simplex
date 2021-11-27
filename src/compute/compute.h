#pragma once

#include "point.h"
#include "simplex.h"
#include "default.h"

#include <vector>
#include <boost/unordered_set.hpp>


template<size_t N>
struct Compute {
    using simplex_t = Simplex<N>;

    Compute(std::vector<point3d>&& points) : points{std::move(points)} {

    }

    std::vector<point3d> points;
    int current_simplices;
    std::array<std::array<bool, N>, N> one_simplex_cache{};
    std::vector<boost::unordered_set<simplex_t>> simplex_cache{};

    template<size_t n, class F>
    void ForEachSimplex(float epsilon, const F& func, bool clear_cache = true);
    void Find1Simplices(float epsilon);

    float Distance2(int i, int j) const {
        float dx = points[i][0] - points[j][0];
        float dy = points[i][1] - points[j][1];
        float dz = points[i][2] - points[j][2];
        return dx * dx + dy * dy + dz * dz;
    }

    template<size_t n>
    std::vector<i32> FindSimplexDrawIndices(float epsilon);
};


template<size_t N>
template<size_t n>
std::vector<i32> Compute<N>::FindSimplexDrawIndices([[maybe_unused]] float epsilon) {
    std::vector<i32> indices = {};
    indices.reserve(n * points.size());
    current_simplices = 0;

    ForEachSimplex<n>(epsilon, [&](simplex_t s) {
        current_simplices++;
        s.ForEachPoint([&](int p) {
            indices.push_back(p);
        });
    });
    return std::move(indices);
}


template<size_t N>
void Compute<N>::Find1Simplices(float epsilon) {
    auto& simplices = simplex_cache[0];

    if (!simplices.empty()) {
        return;
    }

    for (int i = 0; i < points.size(); i++) {
        for (int j = i + 1; j < points.size(); j++) {
            if (Distance2(i, j) <= 4 * epsilon * epsilon) {
                simplices.insert(simplex_t{i, j});
                one_simplex_cache[i][j] = true;
                one_simplex_cache[j][i] = true;
            }
        }
    }
}


template<size_t N>
template<size_t n, class F>
void Compute<N>::ForEachSimplex(float epsilon, const F& func, bool clear_cache) {
    // 0 simplices are always just the points
    if constexpr(n == 0) {
        for (int i = 0; i < points.size(); i++) {
            func(simplex_t{i});
        }
        return;
    }

    // clear simplex cache if needed
    if (clear_cache) {
        simplex_cache = {};
        one_simplex_cache = {};
    }

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

                if (s.ForEachPoint([&](int p) -> bool {
                    // check whether there is a 1-simplex for every point in the simplex
                    if (one_simplex_cache[i][p]) {
                        return true;  // bad simplex, 1-simplex does not exist
                    }
                    return false;  // keep going, 1-simplex exists for this point
                })) {
                    simplices.insert(next);
                    func(next);
                }
            }
        }, false);
    }
}
