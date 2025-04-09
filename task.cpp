#ifndef LOCAL2
#undef _GLIBCXX_DEBUG  // disable run-time bound checking, etc
#pragma GCC optimize("Ofast,inline,unroll-loops,tracer,vpt,split-loops,unswitch-loops")
// #undef __cplusplus
// #define __cplusplus 202002L
#endif

#include <iostream>
#include <unordered_map>
#include <unordered_set>
#include <set>
#include <vector>
#include <algorithm>
#include <ext/pb_ds/assoc_container.hpp>
#include <immintrin.h>

#ifndef LOCAL2
#pragma GCC target( \
    "aes,align-stringops,avx,avx2,bmi,bmi2,crc32,cx16,f16c,fma,fsgsbase,fxsr,hle,ieee-fp,lzcnt,mmx,movbe,mwait,pclmul,popcnt,rdrnd,sahf,sse,sse2,sse3,sse4,sse4.1,sse4.2,ssse3,xsave,xsaveopt")
#endif

#define INLINE inline __attribute__((always_inline))

using namespace __gnu_pbds;

const int MAX_STATES = 4000000;

// 4183357 unique states
// 4142098 without the end states
// stan -> <max depth, sum of hashes> 
// 8 bytes per entry (uint64_t) [truly 59 bits]
// 41259 true end states
// game has depth 60

using namespace std;

typedef uint32_t Board;

constexpr uint32_t blank[9] = {
    0b111111111111111111111111000U,
    0b111111111111111111111000111U,
    0b111111111111111111000111111U,
    0b111111111111111000111111111U,
    0b111111111111000111111111111U,
    0b111111111000111111111111111U,
    0b111111000111111111111111111U,
    0b111000111111111111111111111U,
    0b000111111111111111111111111U,
};

constexpr int32_t neigh[9][4] = {
    {1, 3, 9, 9},
    {0, 4, 2, 9},
    {1, 5, 9, 9},
    {0, 4, 6, 9},
    {1, 3, 5, 7},
    {2, 4, 8, 9},
    {3, 7, 9, 9},
    {4, 6, 8, 9},
    {5, 7, 9, 9},
};

constexpr int32_t neigh_cnt[9] = {
    2, 3, 2,
    3, 4, 3,
    2, 3, 2
};

INLINE uint32_t board_hash(Board board) {
	// /*
	Board b1 = (board & 0b000000111000000111000000111) >> 0;
	Board b2 = (board & 0b000111000000111000000111000) >> 3;
	Board b3 = (board & 0b111000000111000000111000000) >> 6;
	
	b1 = (b1 & 0b111) * 1000000 + ((b1 >> 9) & 0b111) * 1000 + (b1 >> 18);
	b2 = (b2 & 0b111) * 1000000 + ((b2 >> 9) & 0b111) * 1000 + (b2 >> 18);
	b3 = (b3 & 0b111) * 1000000 + ((b3 >> 9) & 0b111) * 1000 + (b3 >> 18);

	return b1 * 100 + b2 * 10 + b3;
	// */

	/*
	uint32_t result = 0;
   
    for (int i = 0; i < 9; i++) {
        result *= 10;
        result += board & 7;
        board >>= 3;
    }

	// cerr << "> " << result << ' ' << res << '\n';
	// assert(result == res);
    
    return result;
	// */
}

INLINE Board horizontal_symmetry(Board board) {
	return ((board & 0b000000000000000000111111111U) << (6 * 3)) |
		   ( board & 0b000000000111111111000000000U) |
		   ((board & 0b111111111000000000000000000U) >> (6 * 3));
}

INLINE Board vertical_symmetry(Board board) {
	return ((board & 0b000000111000000111000000111U) << (3 * 2)) |
		   ( board & 0b000111000000111000000111000U) |
		   ((board & 0b111000000111000000111000000U) >> (3 * 2));
}

const uint32_t zero_solution[40] = {
111111111,704035952,840352818,600875666,50441886,680243700,597686656,584450980,55305380,193520836,521847116,1054388152,518795448,366207036,678967952,476916052,1009258340,592651828,1063467872,400415524,233248832,230461008,245411624,899694236,384163740,888060600,347933640,340717612,73295296,851289228,221286388,375032784,723342020,92414440,745533092,331519112,993643868,72093236,422667876,503115192
};

alignas(32) constexpr uint32_t shifts[32] = {0,3,3,3,3,6,9,9, 9,9,12,12,12,12,12,12, 12,12,12,12,12,15,15,15, 15,18,21,21,21,21,24,0};

alignas(32) constexpr uint32_t masks[32] = {3640,455,28679,29120,29127,229432,28679,1835015,1863680,1863687,3640,229432,232960,233016,14680120,14683648,14683704,14909440,14909496,14913024,14913080,29120,117440960,117469184,117469632,14683648,1863680,117469184,119275520,119304192,14909440,~0U};

alignas(32) constexpr uint32_t index[32] = {7,56,56,56,56,448,3584,3584,3584,3584,28672,28672,28672,28672,28672,28672,28672,28672,28672,28672,28672,229376,229376,229376,229376,1835008,14680064,14680064,14680064,14680064,117440512,0};

constexpr uint32_t holes[4] = {
	(1 << 0) | (1 << 3) | (1 << 6) | (1 << 9),
	(1 << 9) | (1 << 12),
	(1 << 12) | (1 << 15),
	(1 << 15) | (1 << 18) | (1 << 21) | (1 << 24)
};

INLINE void sum_3bit_chunks(__m256i& m) {
	m = _mm256_add_epi32(
		_mm256_and_si256(
			m, 
			_mm256_set1_epi32(0b111000111000111000111000111)
		),
		_mm256_srli_epi32(
			_mm256_and_si256(m, _mm256_set1_epi32(0b000111000111000111000111000)),
			3
		)
	);
	
	m = _mm256_add_epi32(
		_mm256_and_si256(
			m,
			_mm256_set1_epi32(0b111000000111111000000111111)
		),
		_mm256_srli_epi32(
			_mm256_and_si256(m, _mm256_set1_epi32(0b000111111000000111111000000)),
			6
		)
	);
	
	m = _mm256_add_epi32(
		_mm256_and_si256(
			m, 
			_mm256_set1_epi32(0b111000000000000111111111111)
		),
		_mm256_srli_epi32(
			_mm256_and_si256(m, _mm256_set1_epi32(0b000111111111111000000000000)),
			12
		)
	);
	
	m = _mm256_add_epi32(
		_mm256_and_si256(
			m, 
			_mm256_set1_epi32(0b000111111111111111111111111)
		),
		_mm256_srli_epi32(
			_mm256_and_si256(m, _mm256_set1_epi32(0b111000000000000000000000000)),
			24
		)
	);
}

struct State {
	Board board;
	uint32_t dp;
};

uint32_t ans = 0;
State beam[2][MAX_STATES];
int len[2];

INLINE void generate_moves(int nxt, Board const& board, uint32_t ile) {
	uint32_t empty = (((((board & 0b111000111000111000111000111) +
					              0b111000111000111000111000111) &
					             0b1000001000001000001000001000) |
					   (((board & 0b000111000111000111000111000) +
					              0b000111000111000111000111000) &
					             0b0001000001000001000001000000)) >> 3)
							    ^ 0b001001001001001001001001001; // */

	/*
	uint32_t filled_board2 = board;
	uint32_t empty2 = 0;
	for (int i = 0; i < 9; i++) {
		if (((board >> (3 * i)) & 0b111) == 0) {
			filled_board2 |= 0b111 << (3 * i);
			empty2 |= 1 << (3 * i);
		}
	}
	// */
	/*
	if (empty != empty2) {
		cerr << "board: " << board << '\n';
		cerr << "empty: " << empty << " vs " << empty2 << '\n';
		assert(false);
	}
	*/

	if (empty == 0) {
		ans += ile * board_hash(board);
		return;
	}

	uint32_t const filled_board = board | empty | (empty << 1) | (empty << 2);

	// 4 x 256? -> get sum of every submasks
	__m256i const b = _mm256_set1_epi32(filled_board);
	__m256i const p = _mm256_set1_epi32(board);

	int done = 0;
	for (int i = 0; i < 4; i++) if (empty & holes[i]) {
		__m256i const n1 = _mm256_loadu_si256(((__m256i*) masks) + i);
		__m256i const x1 = _mm256_loadu_si256(((__m256i*) index) + i);
		__m256i m1 = _mm256_and_si256(n1, b);
		
		sum_3bit_chunks(m1);	
		__m256i const c1 = _mm256_and_si256(
			_mm256_cmpgt_epi32(_mm256_set1_epi32(7), m1),
			_mm256_cmpgt_epi32(_mm256_set1_epi32(1), _mm256_and_si256(p, x1))
		);
		
		int g1 = _mm256_movemask_ps((__m256) c1);
		
		if (!g1)
			continue;
		/*
		alignas(32) uint32_t sums[8];
		_mm256_store_si256((__m256i*) sums, c1);

		for (int j = 0; j < 8; j++) {
			cerr << sums[j] << ' ';
		}
		cerr << '\n';
		// */

		m1 = _mm256_and_si256(m1, c1);
		m1 = _mm256_sllv_epi32(m1, _mm256_loadu_si256(((__m256i*)shifts) + i));
		m1 = _mm256_or_si256(m1, _mm256_andnot_si256(n1, p));
		
		// c1 -> gives which moves are good
		// m1 -> gives resulting boards
		// cerr << "g1: " << g1 << '\n';

		alignas(32) Board boards[8];
		_mm256_store_si256((__m256i*) boards, m1);

		while (g1) {
			int j = __builtin_ctz(g1);

			beam[nxt][len[nxt]++] = {boards[j], ile};

			// cerr << "? " << j << '\n';
			done |= 1 << shifts[i * 8 + j];
			
			g1 &= g1 - 1;
		}
	}
	
	empty ^= done;
	// cerr << "empty: " << empty << " done: " << done << '\n';
	while (empty) {
		int j = __builtin_ctz(empty);

		beam[nxt][len[nxt]++] = {board | (1 << j), ile};
		// cerr << "left_board: " << board_hash(board | (1 << j)) << '\n';

		empty &= empty - 1;
	}
}

constexpr int X = 1 << 14;

int radix[X];
State tmp[MAX_STATES];

int main() {
    int depth;
    cin >> depth;

	{
		Board board = 0;
		uint16_t empty = 0;
		for (int i = 0; i < 3; i++) {
			for (int j = 0; j < 3; j++) {
				int die_value;
				cin >> die_value;

				board |= die_value << (3 * (i * 3 + j));
				if (die_value == 0) {
					empty |= 1 << (i * 3 + j);
				}
			}
		}

		// /*
		if (board == 0) {
			cout << zero_solution[depth - 1] << '\n';
			return 0;
		}
		// */

		beam[0][len[0]++] = {board, 1};
	}

	for (int d = 0; d < depth; d++) {
		int cur = (d & 1);
		int nxt = cur ^ 1;
		len[nxt] = 0;
	
		const int length = len[cur];

		for (int j = 0; j < X; j++)
			radix[j] = 0;

		cerr << "depth: " << d << " => " << length << '\n';

		for (int j = 0; j < length; j++)
			radix[beam[cur][j].board & (X - 1)]++;
		for (int j = 1; j < X; j++)
			radix[j] += radix[j - 1];
		for (int j = length - 1; j >= 0; j--)
			tmp[--radix[beam[cur][j].board & (X - 1)]] = beam[cur][j];
		for (int j = 0; j < X; j++)
			radix[j] = 0;
		for (int j = 0; j < length; j++)
			radix[(tmp[j].board >> 14) & (X - 1)]++;
		for (int j = 1; j < X; j++)
			radix[j] += radix[j - 1];
		for (int j = length - 1; j >= 0; j--)
			beam[cur][--radix[(tmp[j].board >> 14) & (X - 1)]] = tmp[j];

		for (int j = 1; j < length; j++)
			beam[cur][j].dp += beam[cur][j - 1].dp;

		uint32_t last = 0;
		for (int j = 0; j < length; j++) {
			if (j + 1 == len[cur] || beam[cur][j].board != beam[cur][j + 1].board) {	
				generate_moves(nxt, beam[cur][j].board, beam[cur][j].dp - last);
				last = beam[cur][j].dp;
			}
		}
	}

	for (int i = 0; i < len[depth & 1]; i++) {
		auto const& [b, v] = beam[depth & 1][i];
		ans += v * board_hash(b);
	}
	
    cout << (ans & ((1U << 30) - 1)) << '\n';
   	
	return 0;
}
