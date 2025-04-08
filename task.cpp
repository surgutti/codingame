#ifndef LOCAL
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

#ifndef LOCAL
#pragma GCC target( \
    "aes,align-stringops,avx,avx2,bmi,bmi2,crc32,cx16,f16c,fma,fsgsbase,fxsr,hle,ieee-fp,lzcnt,mmx,movbe,mwait,pclmul,popcnt,rdrnd,sahf,sse,sse2,sse3,sse4,sse4.1,sse4.2,ssse3,xsave,xsaveopt")
#endif

using namespace __gnu_pbds;

const int MAX_STATES = 3684492;

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

inline uint32_t board_hash(Board board) {
    uint32_t result = 0;
    
    for (int i = 0; i < 9; i++) {
        result *= 10;
        result += board & 7;
        board >>= 3;
    }
    
    return result;
}

inline Board horizontal_symmetry(Board board) {
	return ((board & 0b000000000000000000111111111U) << (6 * 3)) |
		   ( board & 0b000000000111111111000000000U) |
		   ((board & 0b111111111000000000000000000U) >> (6 * 3));
}

inline Board vertical_symmetry(Board board) {
	return ((board & 0b000000111000000111000000111U) << (3 * 2)) |
		   ( board & 0b000111000000111000000111000U) |
		   ((board & 0b111000000111000000111000000U) >> (3 * 2));
}

const uint32_t zero_solution[40] = {
111111111,704035952,840352818,600875666,50441886,680243700,597686656,584450980,55305380,193520836,521847116,1054388152,518795448,366207036,678967952,476916052,1009258340,592651828,1063467872,400415524,233248832,230461008,245411624,899694236,384163740,888060600,347933640,340717612,73295296,851289228,221286388,375032784,723342020,92414440,745533092,331519112,993643868,72093236,422667876,503115192
};

alignas(32) constexpr uint32_t shifts[32] = {
	0, 0,
	3, 3, 3,
	6, 6,
	9, 9, 9,
	12, 12, 12, 12,
	18, 18, 18,
	21, 21,
	24, 24, 24,
	27, 27
};

alignas(32) constexpr uint32_t masks[32] = {
/* 0
010
100
000
*/
3640 ,
/* 1
101
000
000
*/
455 ,
/* 2
100
010
000
*/
28679 ,
/* 3
001
010
000
*/
29120 ,
/* 4
101
010
000
*/
29127 ,
/* 5
010
001
000
*/
229432 ,
/* 6
100
010
000
*/
28679 ,
/* 7
100
000
100
*/
1835015 ,
/* 8
000
010
100
*/
1863680 ,
/* 9
100
010
100
*/
1863687 ,
/* 10
010
100
000
*/
3640 ,
/* 11
010
001
000
*/
229432 ,
/* 12
000
101
000
*/
232960 ,
/* 13
010
101
000
*/
233016 ,
/* 14
010
000
010
*/
14680120 ,
/* 15
000
100
010
*/
14683648 ,
/* 16
010
100
010
*/
14683704 ,
/* 17
000
001
010
*/
14909440 ,
/* 18
010
001
010
*/
14909496 ,
/* 19
000
101
010
*/
14913024 ,
/* 20
010
101
010
*/
14913080 ,
/* 21
001
010
000
*/
29120 ,
/* 22
001
000
001
*/
117440960 ,
/* 23
000
010
001
*/
117469184 ,
/* 24
001
010
001
*/
117469632 ,
/* 25
000
100
010
*/
14683648 ,
/* 26
000
010
100
*/
1863680 ,
/* 27
000
010
001
*/
117469184 ,
/* 28
000
000
101
*/
119275520 ,
/* 29
000
010
101
*/
119304192 ,
/* 30
000
001
010
*/
14909440 ,

0
};

inline __m256i sum_3bit_chunks(__m256i m) {
	m = _mm256_add_epi32(
		_mm256_and_si256(m, _mm256_set1_epi32(0b111000111000111000111000111)),
		_mm256_srli_epi32(
			_mm256_and_si256(m, _mm256_set1_epi32(0b000111000111000111000111000)),
			3
		)
	);
	
	m = _mm256_add_epi32(
		_mm256_and_si256(m, _mm256_set1_epi32(0b111000000111111000000111111)),
		_mm256_srli_epi32(
			_mm256_and_si256(m, _mm256_set1_epi32(0b000111111000000111111000000)),
			6
		)
	);
	
	m = _mm256_add_epi32(
		_mm256_and_si256(m, _mm256_set1_epi32(0b111000000000000111111111111)),
		_mm256_srli_epi32(
			_mm256_and_si256(m, _mm256_set1_epi32(0b000111111111111000000000000)),
			12
		)
	);
	
	m = _mm256_add_epi32(
		_mm256_and_si256(m, _mm256_set1_epi32(0b000111111111111111111111111)),
		_mm256_srli_epi32(
			_mm256_and_si256(m, _mm256_set1_epi32(0b111000000000000000000000000)),
			24
		)
	);

	return m;
}

// and with the empty values
void generate_moves(Board const& board) {
	uint32_t empty = 0;
	uint32_t filled_board = board;
	for (int i = 0; i < 9; i++) {
		if (((board >> (3 * i)) & 1) == 0) {
			filled_board |= 0b111 << (3 * i);
			empty |= i;
		}
	}

	if (empty == 0) {
		// end state
		return;
	}
	// 4 x 256? -> get sum of every submasks
	
	__m256i const b = _mm256_set1_epi32(filled_board);
	for (int i = 0; i < 4; i++) {
		__m256i const n1 = _mm256_loadu_si256(((__m256i*)masks) + i);
		__m256i m1 = _mm256_and_si256(n1, b);
		
		m1 = sum_3bit_chunks(m1);
		
		__m256i c1 = _mm256_cmpgt_epi32(_mm256_set1_epi32(7), m1);
		m1 = _mm256_and_si256(m1, c1);
		m1 = _mm256_sllv_epi32(m1, _mm256_loadu_si256(((__m256i*)shifts) + i));
		m1 = _mm256_or_si256(m1, _mm256_andnot_si256(b, n1));
		
		// c1 -> gives which moves are good
		// m1 -> gives resulting boards
		int g1 = _mm256_movemask_ps((__m256) c1);
		
		while (g1) {
			int p = __builtin_ctz(g1);

			Board board = ((Board*) &m1)[p];
			// new board

			g1 &= g1 - 1;
		}
	}

	/*

	__m256i m2 = _m256_loadu_si256(((__m256i*)masks) + 1);
	m2 = _m256_and_si256(m2, b);
	
	m2 = sum_3bit_chunks(m2);
	m2 = _mm256_cmpgt_epi32(_mm256_set1_epi32(7), m2);
	
	__m256i m3 = _m256_loadu_si256(((__m256*)masks) + 2);
	m3 = _m256_and_si256(m3, b);
	
	m3 = sum_3bit_chunks(m3);
	m3 = _mm256_cmpgt_epi32(_mm256_set1_epi32(7), m3);
	
	__m256i m4 = _m256_loadu_si256(((__m256*)masks) + 2);
	m4 = _m256_and_si256(m4, b);
	
	m4 = sum_3bit_chunks(m4);
	m4 = _mm256_cmpgt_epi32(_mm256_set1_epi32(7), m4);

	if (empty & 1) {
				
	}

	if (empty == 0) {

	}
	*/

	/*
	m = ((board | empty) & mask[i]);
	m = (m & 0b111000111000111000111000111) + (m & 0b000111000111000111000111000) / (2 ** 3);
	m = (m & 0b111000000111111000000111111) + (m & 0b000111111000000111111000000) / (2 ** 6);
	m = (m & 0b111000000000000111111111111) + (m & 0b000111111111111000000000000) / (2 ** 12);
	m = (m & 0b000111111111111111111111111) + (m & 0b111000000000000000000000000) / (2 ** 24);

	000 000 
	000 000 
	000 000 
	000 000 
	000 xxx
	
	_m256_add_epi32
	// set 6 where there is 0?


	24 masks / 8 ->
	// mask out the nonempty values?
	__m256 v_empty = _mm256_set_epi16(empty);

	*/
}

pair<Board, uint32_t> beam[2][MAX_STATES];
int len[2];

constexpr int X = 1 << 14;

int radix[X];
pair<Board, uint32_t> tmp[MAX_STATES];

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

		beam[0][len[0]++] = make_pair(board, 1);
	}

	uint32_t ans = 0;

	for (int d = 0; d < depth; d++) {
		int cur = (d & 1);
		int nxt = cur ^ 1;
		len[nxt] = 0;
	
		for (int j = 0; j < X; j++)
			radix[j] = 0;

		cerr << "depth: " << d << " => " << len[cur] << '\n';
		for (int j = 0; j < len[cur]; j++) {
			radix[beam[cur][j].first & (X - 1)]++;
		}

		for (int j = 1; j < X; j++) {
			radix[j] += radix[j - 1];
		}
		assert(radix[X - 1] == len[cur]);

		for (int j = len[cur] - 1; j >= 0; j--) {
			tmp[--radix[beam[cur][j].first & (X - 1)]] = beam[cur][j];
		}

		for (int j = 0; j < X; j++)
			radix[j] = 0;
		
		for (int j = 0; j < len[cur]; j++) {
			radix[(tmp[j].first >> 14) & (X - 1)]++;
		}

		for (int j = 1; j < X; j++) {
			radix[j] += radix[j - 1];
		}

		for (int j = len[cur] - 1; j >= 0; j--) {
			beam[cur][--radix[(tmp[j].first >> 14) & (X - 1)]] = tmp[j];
		}

		for (int j = 1; j < len[cur]; j++) {
			beam[cur][j].second += beam[cur][j - 1].second;
		}

		uint32_t last = 0;
		for (int j = 0; j < len[cur]; j++) {
			if (j + 1 == len[cur] || beam[cur][j].first != beam[cur][j + 1].first) {
				uint32_t ile = beam[cur][j].second - last;
				last = beam[cur][j].second;
			
				{
					Board board = beam[cur][j].first;

					int16_t vals[9];
					int16_t empty = 0;
					for (int i = 0; i < 9; i++) {
						vals[i] = board & 0b111;
						board >>= 2; 
					
						if (vals[i] == 0)
							empty |= 1 << i;
					}

				}

    			const Board board = beam[cur][j].first;

				#define val(i) ((board >> (3 * i)) & 0b111)

				bool is_zero = false;
        		for (int i = 0; i < 9; i++) if (val(i) == 0) { 
					is_zero = true;
					
					bool capture = false;
					for (uint32_t mask = 0; mask < (1U << neigh_cnt[i]); mask++) {
						int cnt = 0, sum = 0;
						Board new_board = board;
						bool ok = true;
					   
						for (uint32_t mm = mask; mm; mm &= mm - 1) {
							int bit = __builtin_ctz(mm); 
						// for (int bit = 0; bit < neigh_cnt[i]; bit++) {
							if (mask >> bit & 1) {
								sum += val(neigh[i][bit]);
								cnt++;
								new_board &= blank[neigh[i][bit]];
							
								if (val(neigh[i][bit]) == 0) {
									ok = false;
								}
							}
						}

						if (cnt >= 2 && sum <= 6 && ok) {
							beam[nxt][len[nxt]++] = make_pair(new_board | (sum << (3 * i)), ile);
							capture = true;
						}
					}

					if (!capture) {
						beam[nxt][len[nxt]++] = make_pair(board | (1U << (3 * i)), ile);
					}
				}

                #undef val

				if (!is_zero) {
					ans += ile * board_hash(board);
				}
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
