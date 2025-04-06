#include <iostream>
#include <unordered_map>
#include <set>

using namespace std;

typedef uint32_t Board;

unordered_map<Board, uint32_t> cache[41];
set<Board> all;

inline uint32_t board_hash(Board board) {
    // cerr << "board> " << board << ' ' << cache.size() << '\n';
    uint32_t result = 0;
    
    // unroll 9 times?
    for (int i = 0; i < 9; i++) {
        result *= 10;
        result += board & 7;
        board >>= 3;
    }
    
    return result;
}

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

// depth == 0 -> bad
// empty == 0 -> good (wektor of prefix sums per depth)??

uint32_t jazda(Board board, uint16_t empty, int depth) {
    // all.insert(board);

    if (depth == 0 || empty == 0) {
        return board_hash(board);
    }

    auto it = cache[depth].find(board);
    if (it != cache[depth].end()) {
        return it->second;
    }
    
    #define val(i) ((board >> (3 * i)) & 0b111)

    uint32_t result = 0;
    for (uint16_t bb = empty; bb; bb &= bb - 1) {
        int i = __builtin_ctz(bb);

        bool capture = false;
        for (uint32_t mask = 0; mask < (1 << neigh_cnt[i]); mask++) {
            int cnt = 0, sum = 0;
            Board new_board = board;
            uint16_t new_empty = empty;
            bool ok = true;
            for (int bit = 0; bit < neigh_cnt[i]; bit++) {
                if (mask >> bit & 1) {
                    sum += val(neigh[i][bit]);
                    new_empty ^= 1 << neigh[i][bit];
                    cnt++;
                    new_board &= blank[neigh[i][bit]];
                
                    if (val(neigh[i][bit]) == 0) {
                        ok = false;
                    }
                }
            }

            if (cnt >= 2 && sum <= 6 && ok) {
                result += jazda(new_board | (sum << (3 * i)), new_empty ^ (1 << i), depth - 1);
                capture = true;
            }
        }

        if (!capture) {
            result += jazda(board | (1U << (3 * i)), empty ^ (1 << i), depth - 1);
        }
    }

    return cache[depth][board] = result;
}

int main() {
    int depth;
    cin >> depth;

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

    // cerr << "depth: " << depth << '\n';
    // cerr << "start: " << board_hash(board) << '\n';
    cout << ((jazda(board, empty, depth) & ((1U << 30) - 1))) << '\n';
	for (int i = 0; i <= depth; i++)
		cerr << cache[i].size() << '\n';

    cerr << "all: " << all.size() << "\n";

    return 0;
}

