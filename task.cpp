#include <iostream>
#include <unordered_map>

using namespace std;

typedef uint32_t Board;

unordered_map<Board, uint32_t> cache;

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

constexpr uint32_t neigh[9][4] = {
    {1, 3, 0, 0},
    {0, 4, 2, 0},
    {1, 5, 0, 0},
    {0, 4, 6, 0},
    {1, 3, 5, 7},
    {2, 4, 8, 0},
    {3, 7, 0, 0},
    {4, 6, 8, 0},
    {5, 7, 0, 0},
};

constexpr int32_t neigh_cnt[9] = {
    2, 3, 2,
    3, 4, 3,
    2, 3, 2
};

uint32_t jazda(Board board, int depth) {
    auto it = cache.find(board);
    if (it != cache.end()) {
        return it->second;
    }
    
    if (depth == 0) {
        // cerr << "board: " << board_hash(board) << '\n';
        return cache[board] = board_hash(board);
    }
    
    #define val(i) ((board >> (3 * i)) & 0b111)

    uint32_t result = 0;
    bool nonzero = false;
    for (int i = 0; i < 9; i++) if (val(i) == 0) {
        bool capture = false;

        for (uint32_t mask = 3; mask < (1 << neigh_cnt[i]); mask++) {
            int cnt = 0, sum = 0;
            Board new_board = board;
            for (int bit = 0; bit < neigh_cnt[i]; bit++) {
                if (mask >> bit & 1) {
                    sum += val(neigh[i][bit]);
                    cnt += val(neigh[i][bit]) > 0;
                    new_board &= blank[neigh[i][bit]];
                }
            }

            if (cnt >= 2 && sum <= 6) {
                // if ((new_board & 0b111000000000000000000000000U) == 0) {
                //     cerr << "??? " << board << ' ' << new_board << ' ' << i << '\n';
                //     cerr << "sum: " << sum << " cnt: " << cnt << '\n';
                //     uint32_t xd = (1U << 27) - 1;
                //     for (int bit = 0; bit < neigh_cnt[i]; bit++) {
                //         if (mask >> bit & 1) {
                //             xd &= blank[neigh[i][bit]];
                //             cerr << neigh[i][bit] << ' ' << blank[neigh[i][bit]] << ' ' << val(neigh[i][bit]) << '\n';
                //         }
                //     }
                //     cerr << board_hash(board) << '\n';
                //     cerr << board_hash(new_board) << '\n';
                //     cerr << xd << '\n';
                // }
                result += jazda(new_board | (sum << (3 * i)), depth - 1);
                capture = true;
            }
        }

        if (!capture) {
            result += jazda(board | (1U << (3 * i)), depth - 1);
        }

        nonzero = true;
    }

    if (!nonzero) {
        result += board_hash(board);
    }

    // return result;
    return cache[board] = result;
}

int main() {
    int depth;
    cin >> depth;

    Board board = 0;
    for (int i = 0; i < 3; i++) {
        for (int j = 0; j < 3; j++) {
            int die_value;
            cin >> die_value;

            board |= die_value << (3 * (i * 3 + j));
        }
    }

    cerr << "depth: " << depth << '\n';
    cerr << "start: " << board_hash(board) << '\n';
    cout << ((jazda(board, depth) & ((1U << 30) - 1))) << '\n';

    return 0;
}
