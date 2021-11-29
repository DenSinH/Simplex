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

    Compute(const std::vector<point3d>& points) : points(points) {

    }

    const std::vector<point3d>& points;

    int current_simplices;

    float last_epsilon = -1;
    std::array<std::array<bool, N>, N> one_simplex_cache{};
    std::vector<boost::unordered_set<simplex_t>> simplex_cache{};

    void ClearSimplexCache(float epsilon);

    template<size_t n, class F>
    void ForEachSimplex(float epsilon, const F& func);
    void Find1Simplices(float epsilon);
    template<size_t n>
    std::vector<i32> FindSimplexDrawIndices(float epsilon);
    std::pair<basis_t, basis_t> FindBZ0(float epsilon);
    template<int n>
    std::pair<basis_t, basis_t> FindBZn(float epsilon);

    float Distance2(int i, int j) const {
        float dx = points[i][0] - points[j][0];
        float dy = points[i][1] - points[j][1];
        float dz = points[i][2] - points[j][2];
        return dx * dx + dy * dy + dz * dz;
    }
};


template<size_t N>
void Compute<N>::ClearSimplexCache(float epsilon) {
    if (epsilon != last_epsilon) {
        simplex_cache = {};
        one_simplex_cache = {};
    }
    last_epsilon = epsilon;
}

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
    // clear cache if needed
    ClearSimplexCache(epsilon);

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


template<size_t N>
std::pair<typename Compute<N>::basis_t, typename Compute<N>::basis_t> Compute<N>::FindBZ0(float epsilon) {
    // low -> column
    // at most N points
    // store low -> simplex
    using b_matrix_t = boost::container::static_vector<std::optional<std::pair<simplex_t, simplex_t>>, N>;
    // higher dimensional simplex -> 1-simplex column (starts as diagonal)
    using z_matrix_t = boost::unordered_map<simplex_t, column_t>;

    // store low -> simplex
    b_matrix_t B(points.size());
    // simplex -> column
    z_matrix_t Z{};
    // basis results (can be constructed while finding them)
    basis_t b_basis{};
    basis_t z_basis{};

    ForEachSimplex<1>(epsilon, [&](const simplex_t s) {
        auto b_col = s;
        auto z_col = column_t{s};
        int low;
        for (low = b_col.FindLow(); b_col && B[low].has_value(); low = b_col.FindLow()) {
            const auto& [low_s, low_col] = B[low].value();
            b_col ^= low_col;

            // low has been found before, so we know that low_s is in Z
            z_col ^= Z.at(low_s);
        }
        if (b_col) {
            B[low] = std::make_pair(s, b_col);

            // this column will never be added to again and is non-zero
            b_basis.push_back(column_t::BoundaryOf(s));

            // this column has not been added to Z yet (low never found)
            Z.emplace(s, z_col);
        }
        else {
            // column will never be read from again in Z, since it ends up being zero
            // it is part of the z_basis though
            z_basis.push_back(z_col);
        }
    });

    // basis for B{n} is all non-zero columns
    // basis for Z{n + 1} is all zero-columns, which we have already kept track of
    return std::make_pair(b_basis, z_basis);
}


template<size_t N>
template<int n>
std::pair<typename Compute<N>::basis_t, typename Compute<N>::basis_t> Compute<N>::FindBZn(float epsilon) {
    if constexpr(n == -1) {
        basis_t z_basis{};
        for (int i = 0; i < points.size(); i++) {
            z_basis.push_back(column_t{simplex_t{i}});
        }
        return std::make_pair(basis_t{}, z_basis);
    }
    else if constexpr(n == 0) {
        return FindBZ0(epsilon);
    }
    else {
        // low -> column
        using b_matrix_t = boost::unordered_map<simplex_t, std::pair<simplex_t, column_t>>;
        // higher dimensional simplex -> column (starts at diagonal)
        using z_matrix_t = boost::unordered_map<simplex_t, column_t>;

        // store low -> simplex
        b_matrix_t B{};
        // simplex -> column
        z_matrix_t Z{};
        // basis results (can be constructed while finding them)
        basis_t b_basis{};
        basis_t z_basis{};

        ForEachSimplex<n + 1>(epsilon, [&](simplex_t s) {
            auto b_col = column_t::BoundaryOf(s);
            auto z_col = column_t{s};
            simplex_t low;
            for (low = b_col.FindLow(); b_col && (B.find(low) != B.end()); low = b_col.FindLow()) {
                const auto& [low_s, low_col] = B.at(low);
                b_col ^= low_col;

                // low has been found before, so we know that low_s is in Z
                z_col ^= Z.at(low_s);
            }
            if (b_col) {
                B.emplace(low, std::make_pair(s, b_col));

                // this column will never be added to again and is non-zero
                b_basis.push_back(b_col);

                // this column has not been added to Z yet (low never found)
                Z.emplace(s, z_col);
            }
            else {
                // column will never be read from again in Z, since it ends up being zero
                // it is part of the z_basis though
                z_basis.push_back(z_col);
            }
        });

        // basis for B{n} is all non-zero columns
        // basis for Z{n + 1} is all zero-columns, which we have already kept track of
        return std::make_pair(b_basis, z_basis);
    }
}