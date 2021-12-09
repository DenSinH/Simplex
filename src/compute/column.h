#pragma once

#include "simplex.h"
#include <boost/container/flat_set.hpp>


template<size_t N>
struct Column {
    using simplex_t = Simplex<N>;
    using vector_t = boost::container::flat_set<std::pair<float, simplex_t>>;

    vector_t data;

    Column<N>() = default;

    explicit Column<N>(float dist, simplex_t s) : data{} {
        data.emplace(dist, s);
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
        // low element is the element that was added last (highest max distance)
        return data.rbegin()->second;
    }
};