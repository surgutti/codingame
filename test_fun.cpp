#include "utils.hpp"
#include "minigames/archery.hpp"
#include "minigames/diving.hpp"
#include "minigames/hurdle_race.hpp"
#include "minigames/roller_skating.hpp"

#include <algorithm>
#include <iostream>
#include <iomanip>
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

void test_hurdle() {

    std::cerr << std::fixed << std::setprecision(9);

    const int track_id = 0;

    HurdleRace::build_fst_snd_dp();

    std::cerr << "fst_dp: " << HurdleRace::fst_dp[track_id][0][0][0][0] << '\n';
    std::cerr << "snd_dp: " << HurdleRace::snd_dp[track_id][0][0][0][0] << '\n';

    int fst = 0;
    int snd = 0;

    for (int rep = 0; rep <= 200'000'000; rep++) {
        HurdleRace hurdle_race;

        hurdle_race.randomize();

        while (!hurdle_race.end) {
            int8_t moves[3] = {randint(0, 3), randint(0, 3), randint(0, 3)};
            hurdle_race.play(moves);
        }

        float places[3];

        hurdle_race.generate_places(places);

        if (places[0] >= places[1]) {
            fst++;
        }
        else {
            snd++;
        }
    }

    std::cerr << "fst: " << double(fst) / (fst + snd) << '\n';
    std::cerr << "snd: " << double(snd) / (fst + snd) << '\n';
}

void test_diving() {
    
    std::cerr << std::fixed << std::setprecision(9);

    Diving::build_dp();

    std::cerr << "fst_dp: " << Diving::fst_dp[15][Diving::id[0][0]][Diving::id[0][0]] << '\n';
    std::cerr << "snd_dp: " << Diving::snd_dp[15][Diving::id[0][0]][Diving::id[0][0]] << '\n';

    int fst = 0;
    int snd = 0;

    float sum = 0;

    const int REP = 20'000'000;

    std::map<int, int> cnt;

    for (int rep = 0; rep < REP; rep++) {
        Diving diving;

        diving.randomize();

        while (!diving.end) {
            int8_t moves[3] = {randint(0, 3), randint(0, 3), randint(0, 3)};
            diving.play(moves);
        }

        float places[3];

        diving.generate_places(places);

        // for (int i = 0; i < 3; i++) {
        //     std::cerr << i << "> " << places[i] << '\n';
        // }

        assert(places[0] == 3 || places[0] == 1 || places[0] == 0);

        if (places[0] >= places[1]) {
            fst++;
        }
        else {
            snd++;
        }

        cnt[places[0]]++;
        sum += places[0];
    }

    std::cerr << "fst: " << double(fst) / (fst + snd) << '\n';
    std::cerr << "snd: " << double(snd) / (fst + snd) << '\n';

    std::cerr << "cnt: ";
    for (auto [a, b] : cnt) {
        std::cerr << a << " => " << b << '\n';
    }

    std::cerr << "avg: " << sum / REP << '\n';

    Diving diving;
    diving.randomize();

    float places[3];
    diving.generate_places(places);

    std::cerr << "avg table: " << places[0] << '\n';
}

void test_archery() {

    std::map<int, int> cnt;

    for (int x = 0; x <= 20; x++) {
        for (int y = 0; y <= 20; y++) {
            cnt[x * x + y * y]++;
        }
    }

    for (auto [a, b] : cnt) {
        std::cerr << a << ' ' << b << '\n';
    }

    std::cerr << "all:" << cnt.size() << '\n';

    return;

    // std::map<std::pair<int, int>, int> cnt;

    // const int REP = 100000;

    // for (int rep = 0; rep < REP; rep++) {
    //     int dx[3] = {0, 0, 0};
    //     int dy[3] = {0, 0, 0};

    //     Archery archery;
    //     archery.randomize();

    //     while (!archery.end) {
    //         int8_t moves[3];

    //         for (int i = 0; i < 3; i++) {
    //             moves[i] = fast_rand() % 4;

    //             if (moves[i] == 0)
    //                 dy[i]++;
    //             if (moves[i] == 1)
    //                 dx[i]--;
    //             if (moves[i] == 2)
    //                 dy[i]--;
    //             if (moves[i] == 3)
    //                 dx[i]++;
    //         }

    //         archery.play(moves);
    //     }

    //     int8_t places[3];
    //     archery.generate_places(places);

    //     for (int i = 0; i < 3; i++) {
    //         std::pair<int, int> key = std::make_pair(abs(dx[i]), abs(dy[i]));

    //         cnt[key] += places[i];
    //     }
    // }

    // for (int i = 0; i <= 15; i++) {
    //     for (int j = 0; j <= 15; j++) {
    //         std::cerr << cnt[std::make_pair(i, j)] << ' ';
    //     }
    //     std::cerr << '\n';
    // }
}

void test_skating() {
    // here the results may differ
    // lets check by how much
    std::cerr << std::fixed << std::setprecision(9);

    const int track_id = 0;

    RollerSkating::build_dp();

    std::cerr << "fst_dp: " << RollerSkating::fst_dp[15][0][2][0][2] << '\n';
    std::cerr << "snd_dp: " << RollerSkating::snd_dp[15][0][2][0][2] << '\n';

    float sum = 0;
    
    const int REP = 20'000'000;

    for (int rep = 0; rep < REP; rep++) {
        RollerSkating roller_skating;

        roller_skating.randomize();

        while (!roller_skating.end) {
            int8_t moves[3] = {randint(0, 3), randint(0, 3), randint(0, 3)};
            roller_skating.play(moves);
        }

        float places[3];

        roller_skating.generate_places(places);

        sum += places[0];
    }

    std::cerr << "avg: " << float(sum) / REP << '\n';

    RollerSkating skating;
    skating.randomize();

    float places[3];
    skating.generate_places(places);
    
    std::cerr << "avg table: " << places[0] << '\n';
}

int main() {
    
    // test_hurdle();
    // test_archery();
    test_diving();
    // test_skating();

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

        // while (times--) {
            tracks.push_back(mask);
        // }
    }

    for (int i = 0; i < (int) tracks.size(); i++) {
        std::cerr << "{" << tracks[i] << "," << i << "},";
    }
    // for (auto x : tracks) {
    //     std::cerr << x << ',';
    // }

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