#include "compute.h"

#include <boost/preprocessor/repetition/repeat.hpp>


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
std::vector<i32> Compute<N>::FindSimplexDrawIndicesImpl([[maybe_unused]] float epsilon) {
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
boost::container::static_vector<std::vector<i32>, 3> Compute<N>::FindSimplexDrawIndices(float epsilon, int n) {
    boost::container::static_vector<std::vector<i32>, 3> result{};
    result.push_back(FindSimplexDrawIndicesImpl<0>(epsilon));
    if (n >= 1) {
        result.push_back(FindSimplexDrawIndicesImpl<1>(epsilon));
        if (n >= 2) {
            result.push_back(FindSimplexDrawIndicesImpl<2>(epsilon));
        }
    }
    return result;
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

#define INSTANTIATE_COMPUTE_METHODS(_, m, n) \
    template std::pair<typename Compute<MIN_POINTS << (n)>::basis_t, typename Compute<MIN_POINTS << (n)>::basis_t> Compute<MIN_POINTS << (n)>::FindBZn<(m) - 1>(float epsilon);

#define INSTANTIATE_COMPUTE(_, n, __) \
    template struct Compute<MIN_POINTS << (n)>; \
    BOOST_PP_REPEAT(MAX_HOMOLOGY_DIM_P1, INSTANTIATE_COMPUTE_METHODS, n)

BOOST_PP_REPEAT(NUM_SHIFTS_P1, INSTANTIATE_COMPUTE, void)