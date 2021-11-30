#pragma once

#include <cstdint>

using u8 = std::uint8_t;
using u16 = std::uint16_t;
using u32 = std::uint32_t;
using u64 = std::uint64_t;
using i8 = std::int8_t;
using i16 = std::int16_t;
using i32 = std::int32_t;
using i64 = std::int64_t;

#define MIN_POINTS 512
#define MAX_POINTS 512
#define NUM_SHIFTS_P1 1
#define MAX_HOMOLOGY_DIM_P1 4
#define MAX_HOMOLOGY_DIM 3
#define MAX_POINT_DIM 8