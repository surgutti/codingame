#include "utils.hpp"

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

int main() {

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