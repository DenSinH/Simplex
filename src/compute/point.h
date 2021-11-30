#pragma once

#include <array>


template<typename T, size_t n>
struct Point {
    static constexpr auto dim = n;

    template<typename... Args>
    Point(Args... args) : data{args...} {

    }

    T& operator[](size_t index) {
        return data[index];
    }

    const T& operator[](size_t index) const {
        return data[index];
    }

private:
    std::array<T, n> data;
};

using point3d = Point<float, 3>;
using point_max = Point<float, 8>;