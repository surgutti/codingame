#ifndef UTILS_HPP
#define UTILS_HPP

#include <cstdint>
#include <cassert>
#include <x86intrin.h>

inline int to_move_index(char c) {
    if (c == 'U') {
        return 0;
    }
    if (c == 'L') {
        return 1;
    }
    if (c == 'D') {
        return 2;
    }
    if (c == 'R') {
        return 3;
    }

    assert(false);
}

inline float fastlogf(const float& x) { union { float f; uint32_t i; } vx = { x }; float y = vx.i; y *= 8.2629582881927490e-8f; return(y - 87.989971088f); }
inline float fastsqrtf(const float& x) { union { int i; float x; } u; u.x = x; u.i = (1 << 29) + (u.i >> 1) - (1 << 22); return(u.x); }
inline float rsqrt_fast(float x) { return _mm_cvtss_f32(_mm_rsqrt_ss(_mm_set_ss(x))); }

inline int pop_count(uint8_t mask) {
    return __builtin_popcount(mask);
}

#endif // UTILS_HPP