#pragma once

#include "simplex.h"
#include <boost/container/flat_set.hpp>


template<size_t N>
struct Column {
    using simplex_t = Simplex<N>;
    using vector_t = boost::container::flat_set<simplex_t>;

    vector_t data;

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
        return data.contains(s);
    }

    operator bool() const {
        return !data.empty();
    }

    Column<N>& operator^=(const Column<N>& other) {
        // in-place, using ordering of the set
        typename vector_t::const_iterator it = data.begin();
        for (const auto s : other.data) {
            while (it != data.end() && (*it < s)) {
                it++;
            }
            if (it == data.end() || (s < *it)) [[likely]] {
                it = data.emplace_hint(it, s);
            }
            else {
                it = data.erase(it);
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