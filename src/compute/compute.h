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
    std::vector<boost::unordered_set<simplex_t>> simplex_cache{};

    template<size_t n, class F>
    void ForEachSimplex(float epsilon, const F& func, bool clear_cache = true);

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
template<size_t n, class F>
void Compute<N>::ForEachSimplex(float epsilon, const F& func, bool clear_cache) {
    if (clear_cache) {
        simplex_cache = {};
    }

    if constexpr(n != 0) {
        if (simplex_cache.size() < n) {
            simplex_cache.resize(n);
        }

        if (!simplex_cache[n - 1].empty()) {
            for (simplex_t s : simplex_cache[n - 1]) {
                func(s);
            }
            return;
        }
    }

    if constexpr(n == 0) {
        for (int i = 0; i < points.size(); i++) {
            func(simplex_t{i});
        }
    }
    else if (n == 1) {
        auto& simplices = simplex_cache[0] = {};
        for (int i = 0; i < points.size(); i++) {
            for (int j = i + 1; j < points.size(); j++) {
                if (Distance2(i, j) <= 4 * epsilon * epsilon) {
                    simplices.insert(simplex_t{i, j});
                    func(simplex_t{i, j});
                }
            }
        }
    }
    else {
        auto& simplices = simplex_cache[n - 1] = {};
        ForEachSimplex<n - 1>(epsilon, [&](simplex_t s) {}, false);
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
                    if (simplex_cache[0].find(simplex_t{p, i}) != simplex_cache[0].end()) {
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
