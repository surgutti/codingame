#include "utils.hpp"
#include "minigames/archery.hpp"

#include <algorithm>
#include <iostream>
#include <cassert>
#include <cmath>
#include <random>
#include <map>

std::mt19937 rng(2137);

int randint(int a, int b) {
    return std::uniform_int_distribution<int>(a, b)(rng);
}

std::string random_track() {
    int startStretch = 3 + randint(0, 4);
    int hurdles = 3 + randint(0, 3);
    int length = 30;

    std::string track;

    for (int i = 0; i < startStretch; i++) {
        track += '.';
    }

    for (int i = 0; i < hurdles; i++) {
        if (randint(0, 1) == 0) {
            track += "#....";
        }
        else {
            track += "#...";
        }
    }

    while ((int) track.size() < length) {
        track += '.';
    }

    return track.substr(0, 29) + ".";
}

void test_diving() {
    int length = 15;

    std::vector<std::pair<int, int>> all;

    all.emplace_back(0, 0);

    for (int i = 0; i < length; i++) {
        std::vector<std::pair<int, int>> nxt;
        
        for (auto [a, b] : all) {
            nxt.emplace_back(a, 0);

            b++;
            nxt.emplace_back(a + b, b);
        }

        all = nxt;
    }

    std::sort(all.begin(), all.end());
    all.erase(std::unique(all.begin(), all.end()), all.end());

    for (auto [a, b] : all) {
        std::cerr << a << ' ' << b << '\n';
    }

    std::cerr << all.size() << '\n';
}

void test_archery() {
    std::map<std::pair<int, int>, int> cnt;

    const int REP = 100000;

    for (int rep = 0; rep < REP; rep++) {
        int dx[3] = {0, 0, 0};
        int dy[3] = {0, 0, 0};

        Archery archery;
        archery.randomize();

        while (!archery.end) {
            int8_t moves[3];

            for (int i = 0; i < 3; i++) {
                moves[i] = fast_rand() % 4;

                if (moves[i] == 0)
                    dy[i]++;
                if (moves[i] == 1)
                    dx[i]--;
                if (moves[i] == 2)
                    dy[i]--;
                if (moves[i] == 3)
                    dx[i]++;
            }

            archery.play(moves);
        }

        int8_t places[3];
        archery.generate_places(places);

        for (int i = 0; i < 3; i++) {
            std::pair<int, int> key = std::make_pair(abs(dx[i]), abs(dy[i]));

            cnt[key] += places[i];
        }
    }

    for (int i = 0; i <= 15; i++) {
        for (int j = 0; j <= 15; j++) {
            std::cerr << cnt[std::make_pair(i, j)] << ' ';
        }
        std::cerr << '\n';
    }
}

int main() {

    test_diving();
    // test_archery();

    return 0;

    std::map<std::string, int> cnt;

    for (int rep = 0; rep < 20000000; rep++) {
        cnt[random_track()]++;
    }

    int min = cnt.begin()->second;

    for (auto [x, y] : cnt) {
        min = std::min(min, y);
    }

    for (auto [x, y] : cnt) {
        std::cerr << x << " => " << y / min << '\n';
    }
    std::cerr << "min: " << min << '\n';
    std::cerr << "cnt: " << cnt.size() << '\n';

    std::vector<uint32_t> tracks;

    for (auto [x, y] : cnt) {
        int times = lround((double) y / min);

        uint32_t mask = 0;
        for (int i = 0; i < 30; i++) {
            if (x[i] == '#') {
                mask |= uint32_t(1) << i;
            }
        }

        while (times--) {
            tracks.push_back(mask);
        }
    }

    for (auto x : tracks) {
        std::cerr << x << ',';
    }

    std::cerr << '\n';
    std::cerr << "> " << tracks.size() << '\n';

    // auto randfloat = [&](double a, double b) {
    //     return std::uniform_real_distribution<double>(a, b)(rng);
    // };

    // long double error = 0;
    // for (int rep = 0; rep < 10000; rep++) {
    //     float x = randfloat(1, 100);

    //     float ans = 1.0 / std::sqrt(x);
    //     float res = rsqrt_fast(x);

    //     error = std::max<long double>(error, fabs(ans - res));

    //     std::cerr << "ERROR: " << error << '\n';
    // }
}