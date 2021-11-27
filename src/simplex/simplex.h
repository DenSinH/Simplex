#pragma once

#include "default.h"

#include <array>
#include <bit>
#include <numeric>
#include <cstring>

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

template<size_t N>
struct Simplex {
    static constexpr size_t bits = sizeof(u64) * 8;
    std::array<u64, (N + bits - 1)/ bits> points;

    Simplex() = default;

    template<typename... Args>
    Simplex(Args... ns) : points{} {
        ((points[ns / bits] |= 1ull << (ns % bits)), ...);
    }

    bool operator==(const Simplex<N>& other) const {
        return std::memcmp(points.data(), other.points.data(), points.size() * sizeof(u64)) == 0;
    }

    Simplex& operator|=(const Simplex<N>& other) {
        for (int i = 0; i < points.size(); i++) {
            points[i] |= other.points[i];
        }
        return *this;
    }

    Simplex operator|(const Simplex<N>& other) {
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

    Simplex operator^(const Simplex<N>& other) {
        Simplex result = *this;
        result ^= other;
        return result;
    }

    int Count() const {
        int count = 0;
        for (const auto& section : points) {
            count += std::popcount(section);
        }
        return count;
    }

    std::pair<int, int> FindLow() const {
        // find lowest 2 set bits
        std::pair<int, int> result;
        int count = 0;
        for (int i = 0; i < points.size(); i++) {
            auto section = points[i];
            if (section) {
                int bit = std::countr_zero(section);
                result.first = count + bit;

                // 2 bits in same section
                if (section ^ (1ull << bit)) {
                    result.second = count + std::countr_zero(section ^ (1ull << bit));
                    return result;
                }
                else {
                    count += bits;
                    for (int j = i + 1; j < points.size(); j++) {
                        section = points[j];
                        if (section) {
                            result.second = count + std::countr_zero(section);
                            return result;
                        }
                        count += bits;
                    }
                }
            }
            count += bits;
        }
        return result;
    }

    template<class F>
    void ForEachPoint(const F& func) {
        int point = 0;
        for (auto section : points) {
            for (; section; section &= ~std::bit_floor(section)) {
                func(point + bits - std::countl_zero(section) - 1);
            }
            point += bits;
        }
    }
};

namespace std {

template<size_t N>
struct hash<Simplex<N>> {
    std::size_t operator()(const Simplex<N>& s) const noexcept {
        return std::accumulate(s.points.begin(), s.points.end(), 0);
    }
};

}