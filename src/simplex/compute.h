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

    ForEachSimplex<n>(epsilon, [&](simplex_t s) {
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
        ForEachSimplex<n - 1>(epsilon, [&](simplex_t s) {
            ForEachSimplex<n - 1>(epsilon, [&](simplex_t t) {
                if (simplices.find(s | t) != simplices.end()) {
                    return;
                }

                if ((s ^ t).Count() == 2) {
                    auto [i, j] = (s ^ t).FindLow();
                    if (Distance2(i ,j) <= 4 * epsilon * epsilon) {
                        simplices.insert(s | t);
                        func(s | t);
                    }
                }
            }, false);
        }, false);
    }
}
