#ifndef RANDOM_HPP
#define RANDOM_HPP

#include "utils.hpp"

#include <cstdint>

static unsigned int g_seed = 2137420;

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

inline int8_t random_move() {
	uint16_t p = fast_rand() & 65535;

	if (p < 20936) {
		return 0;
	}
	else
	if (p < 33291) {
		return 1;
	}
	else
	if (p < 48808) {
		return 2;
	}
	
	return 3;
}

constexpr uint8_t all_permutations[24] = {
    27,
    30,
    39,
    45,
    54,
    57,
    75,
    78,
    99,
    108,
    114,
    120,
    135,
    141,
    147,
    156,
    177,
    180,
    198,
    201,
    210,
    216,
    225,
    228,
};

// inline int8_t random_set_bit(uint8_t move_bitmask) {
// 	uint8_t order = all_permutations[fast_rand() % 24];

// 	if ((move_bitmask >> (order & 3)) & 1)
// 		return order & 3;
// 	order >>= 2;
	
// 	if ((move_bitmask >> (order & 3)) & 1)
// 		return order & 3;
// 	order >>= 2;

// 	if ((move_bitmask >> (order & 3)) & 1)
// 		return order & 3;
// 	order >>= 2;

// 	return order;
// }

#endif
