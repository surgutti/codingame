#ifndef UTILS_HPP
#define UTILS_HPP

#include <cstdint>
#include <cassert>
#include <x86intrin.h>

inline int to_move_index(char c) {
    switch (c) {
    case 'U':
        return 0;
    case 'L':
        return 1;
    case 'D':
        return 2;
    case 'R':
        return 3;
    default:
        assert(false);
        return 0;
    }
}

inline float fastlogf(const float& x) { union { float f; uint32_t i; } vx = { x }; float y = vx.i; y *= 8.2629582881927490e-8f; return(y - 87.989971088f); }
inline float fastsqrtf(const float& x) { union { int i; float x; } u; u.x = x; u.i = (1 << 29) + (u.i >> 1) - (1 << 22); return(u.x); }
inline float rsqrt_fast(float x) { return _mm_cvtss_f32(_mm_rsqrt_ss(_mm_set_ss(x))); }

inline int pop_count(uint8_t mask) {
    return __builtin_popcount(mask);
}

#endif // UTILS_HPP
