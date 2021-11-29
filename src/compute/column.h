#pragma once

#include "simplex.h"
#include <boost/container/set.hpp>


template<size_t N>
struct Column {
    using simplex_t = Simplex<N>;

    boost::container::set<simplex_t> data;

    Column<N>() = default;

    explicit Column<N>(simplex_t s) {
        data.insert(s);
    }

    static Column<N> BoundaryOf(simplex_t s) {
        Column<N> result{};
        s.ForEachPoint([&](int p) {
            // insert all n - 1 simplices by iterating over every point and removing it
            result.data.insert(s ^ simplex_t{p});
        });
        return result;
    }

    bool Contains(const simplex_t& s) {
        return data.find(s) != data.end();
    }

    operator bool() const {
        return !data.empty();
    }

    Column<N>& operator^=(const Column<N>& other) {
        for (const auto s : other.data) {
            if (Contains(s)) [[unlikely]] {
                data.erase(s);
            }
            else {
                data.insert(s);
            }
        }
        return *this;
    }

//    Column<N> operator^(const Column<N>& other) const {
//        auto result = *this;
//        result ^= other;
//        return result;
//    }

    simplex_t FindLow() const {
        return *data.begin();
    }
};