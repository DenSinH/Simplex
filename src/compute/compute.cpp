#include "compute.h"

#include "static_for.h"

#include <thread>
#include <future>
#include <limits>
#include <boost/preprocessor/repetition/repeat.hpp>


template<size_t N>
template<size_t n>
std::vector<i32> Compute<N>::FindSimplexDrawIndicesImpl([[maybe_unused]] float epsilon) {
    std::vector<i32> indices = {};
    indices.reserve(n * points.size());
    this->ComputeBase::current_simplices = 0;

    ForEachSimplex<n>(epsilon, false, [&](float dist, simplex_t s) {
        this->ComputeBase::current_simplices++;
        s.ForEachPoint([&](int p) {
            if constexpr(n < 3) {
                // we can't draw higher dimensional simplices anyway
                indices.push_back(p);
            }
        });
    });
    return std::move(indices);
}

template<size_t N>
boost::container::static_vector<std::vector<i32>, 3> Compute<N>::FindSimplexDrawIndices(float epsilon, int n) {
    boost::container::static_vector<std::vector<i32>, 3> result{};

    result.push_back(FindSimplexDrawIndicesImpl<0>(epsilon));
    if (n >= 1) {
        result.push_back(FindSimplexDrawIndicesImpl<1>(epsilon));
    }
    if (n >= 2) {
        result.push_back(FindSimplexDrawIndicesImpl<2>(epsilon));
    }
    if (n >= 3) {
        detail::static_for<size_t, 0, MAX_HOMOLOGY_DIM_P1>([&](auto i) {
            if (i == n) {
                // just to see how long it actually takes to compute
                FindSimplexDrawIndicesImpl<i>(epsilon);
            }
        });
    }
    return result;
}


template<size_t N>
std::pair<typename Compute<N>::basis_t, typename Compute<N>::basis_t> Compute<N>::FindBZ0(float epsilon, bool ordered) {
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

    ForEachSimplex<1>(epsilon, ordered, [&](float dist, const simplex_t s) {
        auto b_col = s;
        auto z_col = column_t{dist, s};
        int low = b_col.FindLow();
        while (B[low].has_value()) {
            const auto& [low_s, low_col] = B[low].value();
            b_col ^= low_col;

            // low has been found before, so we know that low_s is in Z
            z_col ^= Z.at(low_s);
            if (!b_col) break;
            low = b_col.FindLow();
        }
        if (b_col) {
            B[low] = std::make_pair(s, b_col);

            // this column will never be added to again and is non-zero
            b_basis.emplace_back(s, BoundaryOf<1>(s));

            // this column has not been added to Z yet (low never found)
            Z.emplace(s, z_col);
        }
        else {
            // column will never be read from again in Z, since it ends up being zero
            // it is part of the z_basis though
            z_basis.emplace_back(s, z_col);
        }
    });

    // reduce B basis (unique low)
    z_matrix_t reduced{};
    for (auto& [_, c] : b_basis) {
        auto low = c.FindLow();
        while (reduced.find(low) != reduced.end()) {
            c ^= reduced.at(low);
            low = c.FindLow();
        }
        reduced.emplace(low, c);
    }

    // basis for B{n} is all non-zero columns
    // basis for Z{n + 1} is all zero-columns, which we have already kept track of
    return std::make_pair(b_basis, z_basis);
}


template<size_t N>
template<int n>
std::pair<typename Compute<N>::basis_t, typename Compute<N>::basis_t> Compute<N>::FindBZn(float epsilon, bool ordered) {
    if constexpr(n == -1) {
        basis_t z_basis{};
        for (int i = 0; i < points.size(); i++) {
            z_basis.emplace_back(simplex_t{i}, column_t{0, simplex_t{i}});
        }
        return std::make_pair(basis_t{}, z_basis);
    }
    else if constexpr(n == 0) {
        return FindBZ0(epsilon, ordered);
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

        ForEachSimplex<n + 1>(epsilon, ordered, [&](float dist, simplex_t s) {
            auto b_col = BoundaryOf<n + 1>(s);
            auto z_col = column_t{dist, s};
            simplex_t low = b_col.FindLow();
            while (B.find(low) != B.end()) {
                const auto& [low_s, low_col] = B.at(low);
                b_col ^= low_col;

                // low has been found before, so we know that low_s is in Z
                z_col ^= Z.at(low_s);
                if (!b_col) break;
                low = b_col.FindLow();
            }
            if (b_col) {
                B.emplace(low, std::make_pair(s, b_col));

                // this column will never be added to again and is non-zero
                b_basis.emplace_back(s, b_col);

                // this column has not been added to Z yet (low never found)
                Z.emplace(s, z_col);
            }
            else {
                // column will never be read from again in Z, since it ends up being zero
                // it is part of the z_basis though
                z_basis.emplace_back(s, z_col);
            }
        });

        // basis for B{n} is all non-zero columns
        // basis for Z{n + 1} is all zero-columns, which we have already kept track of
        return std::make_pair(b_basis, z_basis);
    }
}

template<size_t N>
typename Compute<N>::basis_t Compute<N>::FindHBasis(const basis_t& B, const basis_t& Z) const {
    // reduce Z basis to a basis of H
    // basically just sweep the lowest elements
    boost::unordered_map<simplex_t, column_t> reduced{};
    for (auto [_, c] : Z) {
        auto low = c.FindLow();
        while (reduced.find(low) != reduced.end()) {
            c ^= reduced.at(low);
            low = c.FindLow();
        }

#ifndef NDEBUG
        if (!c) [[unlikely]] {
            throw std::runtime_error("Bad basis for Z");
        }
#endif
        reduced.emplace(low, c);
    }

    for (const auto& [_, c] : B) {
        auto low = c.FindLow();
        reduced.erase(low);
    }

    basis_t result{};
    for (auto [s, c] : reduced) {
        result.emplace_back(s, c);
    }
    return std::move(result);
}

template<size_t N>
std::pair<size_t, std::vector<i32>> Compute<N>::FindHBasisDrawIndices(float epsilon, int n) {
    basis_t h_basis;
    detail::static_for<int, 0, MAX_HOMOLOGY_DIM>([&](auto i) {
        if (i == n) {
            auto [b_basis, z_] = FindBZn<i>(epsilon, false);
            auto [b_, z_basis] = FindBZn<i - 1>(epsilon, false);
            h_basis = FindHBasis(b_basis, z_basis);
        }
    });

    std::vector<i32> result{};
    for (const auto& [_, c] : h_basis) {
        for (const auto& [d, s] : c.data) {
            s.ForEachPoint([&result](int p) {
                result.push_back(p);
            });
        }
    }
    return std::make_pair(h_basis.size(), std::move(result));
}

template<size_t N>
std::vector<std::pair<typename Compute<N>::simplex_t, typename Compute<N>::simplex_t>>
Compute<N>::FindBZBasisPairs(const basis_t& B, const basis_t& Z) const {
    // reduce Z basis to a basis of H
    // basically just sweep the lowest elements
    boost::unordered_map<simplex_t, std::pair<simplex_t, column_t>> reduced{};
    for (auto [s, c] : Z) {
        auto low = c.FindLow();
        while (reduced.find(low) != reduced.end()) {
            c ^= reduced.at(low).second;
            low = c.FindLow();
        }

        reduced.emplace(low, std::make_pair(s, c));
    }

    std::vector<std::pair<simplex_t, simplex_t>> result{};
    for (const auto& [s, c] : B) {
        auto low = c.FindLow();
        result.emplace_back(s, reduced.at(low).first);
        reduced.erase(low);
    }

    for (const auto& [s, _] : reduced) {
        // non-paired columns
        result.emplace_back(simplex_t{}, s);
    }
    return result;
}

template<size_t N>
std::array<std::vector<std::pair<float, float>>, MAX_BARCODE_HOMOLOGY + 1>
Compute<N>::FindBarcode(float upper_bound) {
    std::array<std::vector<std::pair<float, float>>, MAX_BARCODE_HOMOLOGY + 1> result{};
    basis_t z_basis = FindBZn<-1>(upper_bound, true).second;

    detail::static_for<int, 0, MAX_HOMOLOGY_DIM>([&](auto i) {
        if (i <= MAX_BARCODE_HOMOLOGY) {
            // compute the basis for B and use the previous basis for Z to compute the next basis for H
            auto [b_basis, z_] = FindBZn<i>(upper_bound, true);
            const auto pairs = FindBZBasisPairs(b_basis, z_basis);
            for (const auto& [b, z] : pairs) {
                float z_dist = i == 0 ? 0 : cache[i - 1].unordered.at(z);
                if (b) [[likely]] {
                    // NOT a basis vector for H
                    result[i].emplace_back(z_dist, cache[i].unordered.at(b));
                }
                else {
                    // basis vector for H
                    result[i].emplace_back(z_dist, std::numeric_limits<float>::infinity());
                }
            }
            // keep next basis for Z
            z_basis = std::move(z_);
        }
    });
    return result;
}

#define INSTANTIATE_COMPUTE_METHODS(_, m, n) \
    template std::pair<typename Compute<MIN_POINTS << (n)>::basis_t, typename Compute<MIN_POINTS << (n)>::basis_t> \
        Compute<MIN_POINTS << (n)>::FindBZn<(m) - 1>(float epsilon, bool ordered);

#define INSTANTIATE_COMPUTE(_, n, __) \
    template struct Compute<MIN_POINTS << (n)>; \
    BOOST_PP_REPEAT(MAX_HOMOLOGY_DIM_P1, INSTANTIATE_COMPUTE_METHODS, n)

BOOST_PP_REPEAT(NUM_SHIFTS_P1, INSTANTIATE_COMPUTE, void)