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
#include <cstring>
#define index sdfsdfsd

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


const uint32_t zero_solution[40] = {
111111111,704035952,840352818,600875666,50441886,680243700,597686656,584450980,55305380,193520836,521847116,1054388152,518795448,366207036,678967952,476916052,1009258340,592651828,1063467872,400415524,233248832,230461008,245411624,899694236,384163740,888060600,347933640,340717612,73295296,851289228,221286388,375032784,723342020,92414440,745533092,331519112,993643868,72093236,422667876,503115192
};

alignas(32) constexpr uint32_t shifts[32] = {0,3,3,3,3,6,9,9, 9,9,12,12,12,12,12,12, 12,12,12,12,12,15,15,15, 15,18,21,21,21,21,24,0};

alignas(32) constexpr uint32_t masks[32] = {3640,455,28679,29120,29127,229432,28679,1835015,1863680,1863687,3640,229432,232960,233016,14680120,14683648,14683704,14909440,14909496,14913024,14913080,29120,117440960,117469184,117469632,14683648,1863680,117469184,119275520,119304192,14909440,~0U};

alignas(32) constexpr uint32_t index[32] = {7,56,56,56,56,448,3584,3584,3584,3584,28672,28672,28672,28672,28672,28672,28672,28672,28672,28672,28672,229376,229376,229376,229376,1835008,14680064,14680064,14680064,14680064,117440512,0};

alignas(32) constexpr uint32_t holes[4] = {
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
	uint32_t dp[8];
};

uint32_t ans = 0;
State beam[2][MAX_STATES];
int len[2];

// permute after applying operatinos
uint16_t permute[8][8] = {
	{0,1,2,3,4,5,6,7,},
	{1,2,3,0,5,6,7,4,},
	{2,3,0,1,6,7,4,5,},
	{3,0,1,2,7,4,5,6,},
	{4,7,6,5,0,3,2,1,},
	{5,4,7,6,1,0,3,2,},
	{6,5,4,7,2,1,0,3,},
	{7,6,5,4,3,2,1,0,},
};

uint32_t lookup90[512] = {
0,1,2,3,4,5,6,7,512,513,514,515,516,517,518,519,1024,1025,1026,1027,1028,1029,1030,1031,1536,1537,1538,1539,1540,1541,1542,1543,2048,2049,2050,2051,2052,2053,2054,2055,2560,2561,2562,2563,2564,2565,2566,2567,3072,3073,3074,3075,3076,3077,3078,3079,3584,3585,3586,3587,3588,3589,3590,3591,262144,262145,262146,262147,262148,262149,262150,262151,262656,262657,262658,262659,262660,262661,262662,262663,263168,263169,263170,263171,263172,263173,263174,263175,263680,263681,263682,263683,263684,263685,263686,263687,264192,264193,264194,264195,264196,264197,264198,264199,264704,264705,264706,264707,264708,264709,264710,264711,265216,265217,265218,265219,265220,265221,265222,265223,265728,265729,265730,265731,265732,265733,265734,265735,524288,524289,524290,524291,524292,524293,524294,524295,524800,524801,524802,524803,524804,524805,524806,524807,525312,525313,525314,525315,525316,525317,525318,525319,525824,525825,525826,525827,525828,525829,525830,525831,526336,526337,526338,526339,526340,526341,526342,526343,526848,526849,526850,526851,526852,526853,526854,526855,527360,527361,527362,527363,527364,527365,527366,527367,527872,527873,527874,527875,527876,527877,527878,527879,786432,786433,786434,786435,786436,786437,786438,786439,786944,786945,786946,786947,786948,786949,786950,786951,787456,787457,787458,787459,787460,787461,787462,787463,787968,787969,787970,787971,787972,787973,787974,787975,788480,788481,788482,788483,788484,788485,788486,788487,788992,788993,788994,788995,788996,788997,788998,788999,789504,789505,789506,789507,789508,789509,789510,789511,790016,790017,790018,790019,790020,790021,790022,790023,1048576,1048577,1048578,1048579,1048580,1048581,1048582,1048583,1049088,1049089,1049090,1049091,1049092,1049093,1049094,1049095,1049600,1049601,1049602,1049603,1049604,1049605,1049606,1049607,1050112,1050113,1050114,1050115,1050116,1050117,1050118,1050119,1050624,1050625,1050626,1050627,1050628,1050629,1050630,1050631,1051136,1051137,1051138,1051139,1051140,1051141,1051142,1051143,1051648,1051649,1051650,1051651,1051652,1051653,1051654,1051655,1052160,1052161,1052162,1052163,1052164,1052165,1052166,1052167,1310720,1310721,1310722,1310723,1310724,1310725,1310726,1310727,1311232,1311233,1311234,1311235,1311236,1311237,1311238,1311239,1311744,1311745,1311746,1311747,1311748,1311749,1311750,1311751,1312256,1312257,1312258,1312259,1312260,1312261,1312262,1312263,1312768,1312769,1312770,1312771,1312772,1312773,1312774,1312775,1313280,1313281,1313282,1313283,1313284,1313285,1313286,1313287,1313792,1313793,1313794,1313795,1313796,1313797,1313798,1313799,1314304,1314305,1314306,1314307,1314308,1314309,1314310,1314311,1572864,1572865,1572866,1572867,1572868,1572869,1572870,1572871,1573376,1573377,1573378,1573379,1573380,1573381,1573382,1573383,1573888,1573889,1573890,1573891,1573892,1573893,1573894,1573895,1574400,1574401,1574402,1574403,1574404,1574405,1574406,1574407,1574912,1574913,1574914,1574915,1574916,1574917,1574918,1574919,1575424,1575425,1575426,1575427,1575428,1575429,1575430,1575431,1575936,1575937,1575938,1575939,1575940,1575941,1575942,1575943,1576448,1576449,1576450,1576451,1576452,1576453,1576454,1576455,1835008,1835009,1835010,1835011,1835012,1835013,1835014,1835015,1835520,1835521,1835522,1835523,1835524,1835525,1835526,1835527,1836032,1836033,1836034,1836035,1836036,1836037,1836038,1836039,1836544,1836545,1836546,1836547,1836548,1836549,1836550,1836551,1837056,1837057,1837058,1837059,1837060,1837061,1837062,1837063,1837568,1837569,1837570,1837571,1837572,1837573,1837574,1837575,1838080,1838081,1838082,1838083,1838084,1838085,1838086,1838087,1838592,1838593,1838594,1838595,1838596,1838597,1838598,1838599,
};

INLINE Board rot(Board const& board) {
	return ((lookup90[(board      ) & 0b111111111])     ) |
		   ((lookup90[(board >>  9) & 0b111111111]) << 3) |
		   ((lookup90[(board >> 18) & 0b111111111]) << 6);
}

INLINE Board sym(Board const& board) {
	return ((board >> 18) |
		    (board << 18) |
		    (board & 0b000000000111111111000000000)) 
		  & 0b111111111111111111;
}

INLINE void update_ans(Board board, uint32_t const* dp) {
	ans += dp[0] * board_hash(board);
	board = rot(board);
	ans += dp[1] * board_hash(board);
	board = rot(board);
	ans += dp[2] * board_hash(board);
	board = rot(board);
	ans += dp[3] * board_hash(board);
	board = sym(board);
	ans += dp[4] * board_hash(board);
	board = rot(board);
	ans += dp[5] * board_hash(board);
	board = rot(board);
	ans += dp[6] * board_hash(board);
	board = rot(board);
	ans += dp[7] * board_hash(board);
}

INLINE void generate_moves(int const nxt, Board const& board, uint32_t const *ile) {
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
		update_ans(board, ile);
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

			beam[nxt][len[nxt]].board = boards[j];
			memcpy(beam[nxt][len[nxt]].dp, ile, 8 * sizeof(uint32_t));
			len[nxt]++;

			// cerr << "? " << j << '\n';
			done |= 1 << shifts[i * 8 + j];
			
			g1 &= g1 - 1;
		}
	}
	
	empty ^= done;
	// cerr << "empty: " << empty << " done: " << done << '\n';
	while (empty) {
		int j = __builtin_ctz(empty);

		beam[nxt][len[nxt]].board = board | (1 << j);
		memcpy(beam[nxt][len[nxt]].dp, ile, 8 * sizeof(uint32_t));
		len[nxt]++;
		// cerr << "left_board: " << board_hash(board | (1 << j)) << '\n';

		empty &= empty - 1;
	}
}

constexpr int X = 1 << 14;
constexpr int Y = 1 << 13;

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

		/*
		if (board == 0) {
			cout << zero_solution[depth - 1] << '\n';
			return 0;
		}
		// */

		beam[0][len[0]++] = {board, {1, 0, 0, 0, 0, 0, 0, 0}};
	}

	for (int d = 0; d < depth; d++) {
		const int cur = (d & 1);
		const int nxt = cur ^ 1;
		len[nxt] = 0;
	
		const int length = len[cur];
		
		uint32_t tmp_p[8];
		for (int j = 0; j < length; j++) {
			Board board = beam[cur][j].board;
			Board best = board;
			int best_i = 0;

			#define UPD(k) if (best > board) { best = board, best_i = k; }

			board = rot(board); UPD(1)
			board = rot(board); UPD(2)
			board = rot(board); UPD(3)
			board = sym(board); UPD(4)
			board = rot(board); UPD(5)
			board = rot(board); UPD(6)
			board = rot(board); UPD(7)
			
			// apply permutation
			if (best_i != 0) {
				beam[cur][j].board = best;				
				memcpy(tmp_p, beam[cur][j].dp, 8 * sizeof(uint32_t));

				for (int k = 0; k < 8; k++) {
					beam[cur][j].dp[permute[best_i][k]] = tmp_p[k];
				}
			}
		}

		// #ifdef LOCAL
			cerr << "depth: " << d << " => " << length << '\n';
		// #endif

		for (int j = 0; j < X; j++)
			radix[j] = 0;
		for (int j = 0; j < length; j++)
			radix[beam[cur][j].board & (X - 1)]++;
		for (int j = 1; j < X; j++)
			radix[j] += radix[j - 1];
		for (int j = length - 1; j >= 0; j--)
			tmp[--radix[beam[cur][j].board & (X - 1)]] = beam[cur][j];
		for (int j = 0; j < Y; j++)
			radix[j] = 0;
		for (int j = 0; j < length; j++)
			radix[(tmp[j].board >> 14) & (Y - 1)]++;
		for (int j = 1; j < Y; j++)
			radix[j] += radix[j - 1];
		for (int j = length - 1; j >= 0; j--)
			beam[cur][--radix[(tmp[j].board >> 14) & (Y - 1)]] = tmp[j];

		for (int j = 1; j < length; j++)
			for (int k = 0; k < 8; k++)
				beam[cur][j].dp[k] += beam[cur][j - 1].dp[k];

		uint32_t last[8] = {0};
		for (int j = 0; j < length; j++) {
			if (j + 1 == len[cur] || beam[cur][j].board != beam[cur][j + 1].board) {
				uint32_t now[8] = {0};
				for (int k = 0; k < 8; k++) {
					now[k] = beam[cur][j].dp[k] - last[k];
					last[k] = beam[cur][j].dp[k];
				}
				generate_moves(nxt, beam[cur][j].board, now);
			}
		}
	}

	for (int i = 0; i < len[depth & 1]; i++) {
		update_ans(beam[depth & 1][i].board, 
				   beam[depth & 1][i].dp);
	}
	
    cout << (ans & ((1U << 30) - 1)) << '\n';
   	
	return 0;
}
