#ifndef RANDOM_HPP
#define RANDOM_HPP

#include "utils.hpp"

static unsigned int g_seed = 2137;

inline void fast_srand(int seed) {
	g_seed = seed;
}

inline int fast_rand() {
	unsigned int z = (g_seed += 0x39A3689AU);
	z = (z ^ (z >> 14)) * 0x108E8AB8;
	z = (z ^ (z >> 13)) * 0x2CD01FD6;
	return int((z ^ (z >> 15)) >> 1);
}

inline int fast_rand(int a, int b) {
    return a + fast_rand() % (b - a + 1);
}

#endif
