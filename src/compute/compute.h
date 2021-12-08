#pragma once

#include "point.h"
#include "simplex.h"
#include "column.h"
#include "default.h"

#include <vector>
#include <boost/container/flat_set.hpp>
#include <boost/unordered_set.hpp>
#include <boost/unordered_map.hpp>
#include <boost/container/static_vector.hpp>


struct ComputeBase {
    using point_t = point_max;

    ComputeBase(const std::vector<point_t>& points) : points(points) {

    }

    virtual ~ComputeBase() = default;

    const std::vector<point_t>& points;
    int current_simplices = 0;

    virtual boost::container::static_vector<std::vector<i32>, 3> FindSimplexDrawIndices(float epsilon, int n) = 0;
    virtual std::pair<size_t, std::vector<i32>> FindHBasisDrawIndices(float epsilon, int n) = 0;
};



template<size_t N>
struct Compute final : ComputeBase {
    using simplex_t = Simplex<N>;
    using column_t = Column<N>;
    using basis_t = std::vector<column_t>;

    struct SimplexCache {
        float max_epsilon = {};
        boost::container::flat_set<simplex_t> ordered{};
        boost::unordered_set<simplex_t> unordered{};
    };

    Compute(const std::vector<point_t>& points) : ComputeBase(points) {

    }

    ~Compute() final = default;

    std::vector<SimplexCache> cache{};


    template<size_t n, class F>
    void ForEachSimplex(float epsilon, const F& func);

    // find simplex draw indices for frontend given for all dimensions lower than the given dimension
    // and lower then 3 (we cannot draw 3-dimensional solids anyway
    boost::container::static_vector<std::vector<i32>, 3> FindSimplexDrawIndices(float epsilon, int n) final;

    // find simplex draw indices for the basis of the homology group of the given dimension
    std::pair<size_t, std::vector<i32>> FindHBasisDrawIndices(float epsilon, int n) final;

    // reduce a basis for Z to a basis of H given a basis for B
    basis_t FindHBasis(const basis_t& B, const basis_t& Z) const;

    // find a vector of H bases for all dimensions lower or equal to the given dimension
    std::vector<basis_t> FindHBases(float epsilon, int n);

    // find a barcode given a range of epsilons
    std::array<std::vector<std::vector<float>>, MAX_BARCODE_HOMOLOGY + 1> FindBarcode(float lower_bound, float upper_bound, float de);

private:
    template<size_t n>
    std::vector<i32> FindSimplexDrawIndicesImpl(float epsilon);

    // find the basis for B{0} and Z{1}
    // this is a special (optimized) method for the one below
    std::pair<basis_t, basis_t> FindBZ0(float epsilon);

    // find the basis for B{n} and Z{n + 1}
    template<int n>
    std::pair<basis_t, basis_t> FindBZn(float epsilon);

    template<size_t n>
    void FindnSimplices(float epsilon);

    // find the distance between 2 points given their indices
    float Distance2(int i, int j) const {
        float dist = 0;
        for (int c = 0; c < point_t::dim; c++) {
            const float dx = points[i][c] - points[j][c];
            dist += dx * dx;
        }
        return dist;
    }

    template<int n>
    Column<N> BoundaryOf(simplex_t s) {
        Column<N> result{};
        s.ForEachPoint([&](int p) {
            // insert all n - 1 simplices by iterating over every point and removing it
            if constexpr(n > 1) {
                // we need to find the right max_dist too, this is probably faster than calculating it
                auto face = cache[n - 2].unordered.find(s ^ simplex_t{p});
                result.data.insert(*face);
            }
            else {
                // for 1-simplices, the boundary consists of 0-simplices with 0 max_dist
                result.data.insert(simplex_t{p});
            }
        });
        return result;
    }
};

template<size_t N>
template<size_t n>
void Compute<N>::FindnSimplices(float epsilon) {
    if constexpr(n == 0) {
        return;
    }

    if (cache.size() < n) {
        cache.resize(n);
    }

    if (epsilon <= cache[n - 1].max_epsilon) {
        return;
    }
    const float prev_epsilon = cache[n - 1].max_epsilon;
    cache[n - 1].max_epsilon = epsilon;

    // 1 simplices are special since we can use them for the higher order simplices
    if constexpr(n == 1) {
        auto& ordered_simplices = cache[0].ordered;
        auto& unordered_simplices = cache[0].unordered;

        for (int i = 0; i < points.size(); i++) {
            for (int j = i + 1; j < points.size(); j++) {
                const float dist2 = Distance2(i, j);
                if (dist2 <= 4 * epsilon * epsilon) {
                    auto s = simplex_t{i, j};
                    s.max_dist = dist2;
                    ordered_simplices.emplace(s);
                    unordered_simplices.insert(s);
                }
            }
        }
    }
    else {
        auto& unordered_simplices = cache[n - 1].unordered;
        auto& ordered_simplices = cache[n - 1].ordered;

        FindnSimplices<n - 1>(epsilon);

        simplex_t lower_bound = simplex_t{};
        lower_bound.max_dist = 4 * prev_epsilon * prev_epsilon;
        for (auto it = cache[n - 2].ordered.lower_bound(lower_bound); it != cache[n - 2].ordered.end(); it++) {
            const auto s = *it;
            // try every other point
            for (int i = 0; i < points.size(); i++) {
                if (s[i]) [[unlikely]] {
                    // point is already contained in the simplex
                    continue;
                }
                auto next = s | simplex_t{i};
                if (unordered_simplices.find(next) != unordered_simplices.end()) {
                    // simplex is already found
                    continue;
                }

                float dist = s.max_dist;
                if (!s.ForEachPoint([&](int p) -> bool {
                    // check whether there is a 1-simplex for every point in the simplex
                    dist = std::max(dist, Distance2(i, p));
                    if (dist > 4 * epsilon * epsilon) {
                        return true;  // bad simplex, 1-simplex does not exist
                    }
                    return false;  // keep going, 1-simplex exists for this point
                })) {
                    next.max_dist = dist;
                    ordered_simplices.emplace(next);
                    unordered_simplices.insert(next);
                }
            }
        }
    }
}

template<size_t N>
template<size_t n, class F>
void Compute<N>::ForEachSimplex(float epsilon, const F& func) {
    // 0 simplices are always just the points
    if constexpr(n == 0) {
        for (int i = 0; i < points.size(); i++) {
            func(simplex_t{i});
        }
    }
    else {
        FindnSimplices<n>(epsilon);
        for (auto& s : cache[n - 1].ordered) {
            if (s.max_dist <= 4 * epsilon * epsilon) {
                func(s);
            }
        }
    }
}