#pragma once

#include "default.h"

#include <array>
#include <bit>
#include <numeric>
#include <cstring>
#include <boost/range/combine.hpp>
#include <boost/foreach.hpp>

/*
 * Idea: have a bitset of n bits, each representing a point.
 * An n-simplex contains n points, so it has n bits set.
 * To find simplices one level higher, we can use 2 approaches:
 *  - The naive approach: loop over all points, for each point over every other point etc. (n nested loops)
 *    and check whether all mutual distances are smaller than the given radius (Vietoris Rips)
 *  - Take 2 existing n - 1 simplices, and calculate S1 ^ S2. If S1 ^ S2 has 2 bits set, there are only
 *    2 points that differ between them, so they are a candidate for a new simplex.
 *    If the distance between the 2 different points is smaller than the given radius (Vietoris Rips),
 *    the new simplex (S1 | S2) is again a valid simplex.
 * */

namespace detail {

/*
 * For getting function argument types.
 * */

template<typename... Args>
struct pack {

};

template<typename Callable> struct func;

template<typename R, typename... Args>
struct func<R(Args...)> {
    using args_t = pack<Args...>;
    using return_t = R;
};

template<typename R, typename... Args>
struct func<R (*)(Args...)> {
    using args_t = pack<Args...>;
    using return_t = R;
};

template<typename R, typename C, typename... Args>
struct func<R (C::*)(Args...)> {
    using args_t = pack<Args...>;
    using return_t = R;
};

template<typename R, typename C, typename... Args>
struct func<R (C::*)(Args...) const> {
    using args_t = pack<Args...>;
    using return_t = R;
};

template<typename Callable>
struct func {
    using args_t = typename func<decltype(&Callable::operator())>::args_t;
    using return_t = typename func<decltype(&Callable::operator())>::return_t;
};

}

template<size_t N>
struct Simplex {
    static constexpr size_t bits = sizeof(u64) * 8;
    std::array<u64, (N + bits - 1) / bits> points;

    Simplex() = default;

    template<typename... Args>
    Simplex(Args... ns) : points{} {
        ((points[ns / bits] |= 1ull << (ns % bits)), ...);
    }

    bool operator==(const Simplex<N>& other) const {
        return std::memcmp(points.data(), other.points.data(), points.size() * sizeof(u64)) == 0;
    }

    bool operator<(const Simplex<N>& other) const {
        // lexographic ordering
        int i = 0, j = 0;
        u64 si, sj;
        BOOST_FOREACH(boost::tie(si, sj), boost::combine(points, other.points)) {
            while (si && sj) {
                if (std::countr_zero(si) < std::countr_zero(sj)) {
                    return true;
                }
                else if (std::countr_zero(si) == std::countr_zero(sj)) {
                    const auto lowest_bit = 1ull << std::countr_zero(si);
                    si ^= lowest_bit;
                    sj ^= lowest_bit;
                }
                else {
                    return false;
                }
            }
            if (si) {
                return true;
            }
            if (sj) {
                return false;
            }
        }
        return false;
    }

    operator bool() const {
        return std::any_of(points.begin(), points.end(), [](const u64& v) { return v != 0; });
    }

    bool operator[](size_t index) const {
        return (points[index / bits] & 1ull << (index % bits)) != 0;
    }

    Simplex& operator|=(const Simplex<N>& other) {
        for (int i = 0; i < points.size(); i++) {
            points[i] |= other.points[i];
        }
        return *this;
    }

    Simplex operator|(const Simplex<N>& other) const {
        Simplex result = *this;
        result |= other;
        return result;
    }

    Simplex& operator^=(const Simplex<N>& other) {
        for (int i = 0; i < points.size(); i++) {
            points[i] ^= other.points[i];
        }
        return *this;
    }

    Simplex operator^(const Simplex<N>& other) const {
        Simplex result = *this;
        result ^= other;
        return result;
    }

    int Count() const {
        return std::accumulate(points.begin(), points.end(), 0, std::popcount<u64>);
    }

    int FindLow() const {
        int count = 0;
        for (auto section : points) {
            if (section) {
                int bit = std::countr_zero(section);
                return count + bit;
            }
            count += bits;
        }
        return -1;
    }

    std::pair<int, int> FindLow2() const {
        // find lowest 2 set bits
        std::pair<int, int> result;
        int count = 0;
        int i;
        for (i = 0; i < points.size(); i++) {
            auto section = points[i];
            if (section) {
                int bit = std::countr_zero(section);
                result.first = count + bit;

                // 2 bits in same section
                if (section ^ (1ull << bit)) {
                    result.second = count + std::countr_zero(section ^ (1ull << bit));
                    return result;
                }
                break;
            }
            count += bits;
        }

        count += bits;
        for (int j = i + 1; j < points.size(); j++) {
            auto section = points[j];
            if (section) {
                result.second = count + std::countr_zero(section);
                return result;
            }
            count += bits;
        }

        return result;
    }

    template<class F, typename T = typename detail::func<F>::return_t>
    T ForEachPoint(const F& func) const {
        int point = 0;
        for (auto section : points) {
            for (; section; section &= ~std::bit_floor(section)) {
                if constexpr(std::is_same_v<T, void>) {
                    func(point + bits - std::countl_zero(section) - 1);
                }
                else {
                    T value = func(point + bits - std::countl_zero(section) - 1);
                    if (value) {
                        return value;
                    }
                }
            }
            point += bits;
        }
        if constexpr(!std::is_same_v<T, void>) {
            return T{};
        }
    }
};


template<size_t N>
std::size_t hash_value(const Simplex<N>& s) noexcept {
    // this hash value works for up to log_N(2^64) points
    // if we use N = 512, then this gives us about 64 / 9 = 7 points
    // after that the hash will slow down somewhat
    size_t result = 0;
    size_t multiplier = 1;
    s.ForEachPoint([&](int p) {
        result += p * multiplier;
        multiplier *= N;
    });
    return result;
}
