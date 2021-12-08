#include "compute.h"

#include "static_for.h"

#include <thread>
#include <future>
#include <boost/preprocessor/repetition/repeat.hpp>


template<size_t N>
template<size_t n>
std::vector<i32> Compute<N>::FindSimplexDrawIndicesImpl([[maybe_unused]] float epsilon) {
    std::vector<i32> indices = {};
    indices.reserve(n * points.size());
    this->ComputeBase::current_simplices = 0;

    ForEachSimplex<n>(epsilon, [&](simplex_t s) {
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
            b_basis.push_back(BoundaryOf<1>(s));

            // this column has not been added to Z yet (low never found)
            Z.emplace(s, z_col);
        }
        else {
            // column will never be read from again in Z, since it ends up being zero
            // it is part of the z_basis though
            z_basis.push_back(z_col);
        }
    });

    // reduce B basis (unique low)
    z_matrix_t reduced{};
    for (auto& c : b_basis) {
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
            auto b_col = BoundaryOf<n + 1>(s);
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

template<size_t N>
typename Compute<N>::basis_t Compute<N>::FindHBasis(const basis_t& B, const basis_t& Z) const {
    // reduce Z basis to a basis of H
    // basically just sweep the lowest elements
    boost::unordered_map<simplex_t, column_t> reduced{};
    for (column_t c : Z) {
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

    for (const column_t& c : B) {
        auto low = c.FindLow();
        reduced.erase(low);
    }

    basis_t result{};
    for (auto [s, c] : reduced) {
        result.push_back(c);
    }
    return std::move(result);
}

template<size_t N>
std::pair<size_t, std::vector<i32>> Compute<N>::FindHBasisDrawIndices(float epsilon, int n) {
    basis_t h_basis;
    detail::static_for<int, 0, MAX_HOMOLOGY_DIM>([&](auto i) {
        if (i == n) {
            auto [b_basis, z_] = FindBZn<i>(epsilon);
            auto [b_, z_basis] = FindBZn<i - 1>(epsilon);
            h_basis = FindHBasis(b_basis, z_basis);
        }
    });

    std::vector<i32> result{};
    for (const auto& c : h_basis) {
        for (const auto& s : c.data) {
            s.ForEachPoint([&result](int p) {
                result.push_back(p);
            });
        }
    }
    return std::make_pair(h_basis.size(), std::move(result));
}

template<size_t N>
std::vector<typename Compute<N>::basis_t> Compute<N>::FindHBases(float epsilon, int n) {
    std::vector<basis_t> result{};
    basis_t z_basis = FindBZn<-1>(epsilon).second;

    detail::static_for<int, 0, MAX_HOMOLOGY_DIM>([&](auto i) {
        if (i <= n) {
            // compute the basis for B and use the previous basis for Z to compute the next basis for H
            auto [b_basis, z_] = FindBZn<i>(epsilon);
            result.push_back(FindHBasis(b_basis, z_basis));
            // keep next basis for Z
            z_basis = std::move(z_);
        }
    });

    return result;
}

template<size_t N>
std::array<std::vector<std::vector<float>>, MAX_BARCODE_HOMOLOGY + 1> Compute<N>::FindBarcode(float lower_bound, float upper_bound, float de) {
    const auto num_workers = std::thread::hardware_concurrency() / 2;

    // create worker Compute instances
    std::vector<std::unique_ptr<Compute<N>>> workers{};
    workers.reserve(num_workers);
    for (int i = 0; i < num_workers; i++) {
        workers.push_back(std::make_unique<Compute<N>>(points));
    }

    // futures and labeled results
    // when we return we do not care what the low value was anymore, but for collecting the results we do
    std::vector<std::pair<float, std::future<std::vector<basis_t>>>> futures;
    futures.resize(num_workers);
    std::array<boost::unordered_map<simplex_t, std::vector<float>>, MAX_BARCODE_HOMOLOGY + 1> labeled_results{};

    // keep track of epsilon per basis vector low simplex
    auto parse_results = [&](float eps, std::vector<basis_t>& bases) {
        for (int dim = 0; dim < bases.size(); dim++) {
            std::printf("Got basis for H%d at eps = %f of size %llu\n", dim, eps, bases[dim].size());
            for (auto& c : bases[dim]) {
                const auto low = c.FindLow();

                if (labeled_results[dim].find(low) == labeled_results[dim].end()) {
                    labeled_results[dim][low] = {};
                }
                labeled_results[dim].at(low).push_back(eps);
            }
        }
    };

    // with optimized caching this will be way faster
    float epsilon = upper_bound;
    while (epsilon > lower_bound) {
        int i;
        for (i = 0; i < num_workers; i++) {
            if (futures[i].second.valid()) {
                // get results
                if (futures[i].second.wait_for(std::chrono::milliseconds(0)) == std::future_status::ready) {
                    auto& [eps, fut] = futures[i];
                    auto bases = fut.get();
                    parse_results(eps, bases);
                }
            }

            // spawn worker for new epsilon
            if (!futures[i].second.valid()) {
                std::printf("Launching for %f\n", epsilon);
                futures[i] = std::make_pair(epsilon, std::async(
                        std::launch::async, &Compute<N>::FindHBases, workers[i].get(), epsilon, MAX_BARCODE_HOMOLOGY
                ));
                break;
            }
        }

        // no available worker found
        if (i == num_workers) {
            // wait for worker (choice is rather arbitrary)
            futures[0].second.wait();
        }
        else {
            epsilon -= de;
        }
    }

    // get leftover results
    for (auto& [eps, fut] : futures) {
        if (fut.valid()) {
            fut.wait();
            auto bases = fut.get();
            parse_results(eps, bases);
        }
    }

    // convert labeled_results into unlabeled results
    std::array<std::vector<std::vector<float>>, MAX_BARCODE_HOMOLOGY + 1> results{};
    for (int dim = 0; dim < labeled_results.size(); dim++) {
        for (auto&& [k, v] : labeled_results[dim]) {
            results[dim].push_back(v);
        }
    }

    return results;
}

#define INSTANTIATE_COMPUTE_METHODS(_, m, n) \
    template std::pair<typename Compute<MIN_POINTS << (n)>::basis_t, typename Compute<MIN_POINTS << (n)>::basis_t> Compute<MIN_POINTS << (n)>::FindBZn<(m) - 1>(float epsilon);

#define INSTANTIATE_COMPUTE(_, n, __) \
    template struct Compute<MIN_POINTS << (n)>; \
    BOOST_PP_REPEAT(MAX_HOMOLOGY_DIM_P1, INSTANTIATE_COMPUTE_METHODS, n)

BOOST_PP_REPEAT(NUM_SHIFTS_P1, INSTANTIATE_COMPUTE, void)