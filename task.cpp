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

pair<Board, uint32_t> beam[2][MAX_STATES];
int len[2];

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

		if (board == 0) {
			cout << zero_solution[depth - 1] << '\n';
			return 0;
		}

		beam[0][len[0]++] = make_pair(board, 1);
	}

	uint32_t ans = 0;

	for (int d = 0; d < depth; d++) {
		int cur = (d & 1);
		int nxt = cur ^ 1;
		len[nxt] = 0;
		
		cerr << "depth: " << d << " => " << len[cur] << '\n';
		sort(beam[cur], beam[cur] + len[cur]);
		reverse(beam[cur], beam[cur] + len[cur]);
		
		for (int j = 0; j < min<int>(20, len[cur]); j++) {
			if (j == 0 || beam[cur][j].first != beam[cur][j - 1].first) {
				// cerr << "> " << beam[cur][j].second << ' ' << board_hash(beam[cur][j].first) << '\n';
			}
		}

		for (int j = 1; j < len[cur]; j++) {
			beam[cur][j].second += beam[cur][j - 1].second;
		}

		uint32_t last = 0;
		for (int j = 0; j < len[cur]; j++) {
			if (j + 1 == len[cur] || beam[cur][j].first != beam[cur][j + 1].first) {
				uint32_t ile = beam[cur][j].second - last;
				last = beam[cur][j].second;
			
    			const Board board = beam[cur][j].first;

				#define val(i) ((board >> (3 * i)) & 0b111)

				bool is_zero = false;
        		for (int i = 0; i < 9; i++) if (val(i) == 0) { 
					is_zero = true;
					
					bool capture = false;
					for (uint32_t mask = 0; mask < (1 << neigh_cnt[i]); mask++) {
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
