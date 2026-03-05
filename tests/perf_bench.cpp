#include "brain.hpp"

#include <chrono>
#include <cstdint>
#include <cstdlib>
#include <iostream>

namespace {

State make_seed_state() {
    State s{};
    for (int i = 0; i < PLAYER_COUNT; ++i) {
        s.hurdles_score[i] = 0.0f;
        s.archery_score[i] = 0.0f;
        s.skating_score[i] = 0.0f;
        s.divings_score[i] = 0.0f;
    }

    const State::InputGpuArray gpu = {
        ".......#...#...#...#...#......",
        "821141321141",
        "DRUL",
        "LLLDDUDLLDDU"
    };

    const State::InputRegArray reg = {{
        {{2, 3, 1, 0, 0, 0, 0}},
        {{5, -5, 7, -7, 3, -7, 0}},
        {{2, 3, 2, 2, 2, 3, 14}},
        {{1, 0, 0, 1, 0, 0, 0}}
    }};

    s.init(gpu, reg);
    s.turn = 0;
    return s;
}

} // namespace

int main(int argc, char** argv) {
    int runs = 200;
    int timeout_ms = 5;
    if (argc > 1) {
        runs = std::atoi(argv[1]);
    }
    if (argc > 2) {
        timeout_ms = std::atoi(argv[2]);
    }

    fast_srand(2137420);
    Hurdles::build_dp();

    const State root_state = make_seed_state();
    Brain brain;
    uint64_t total_root_vis = 0;

    const auto t0 = std::chrono::steady_clock::now();
    for (int i = 0; i < runs; ++i) {
        timer.start();
        brain.run(root_state, timeout_ms);
        total_root_vis += brain.roots[0]->vis;
    }
    const auto t1 = std::chrono::steady_clock::now();
    auto elapsed_ms = std::chrono::duration_cast<std::chrono::milliseconds>(t1 - t0).count();
    if (elapsed_ms <= 0) {
        elapsed_ms = 1;
    }

    std::cout << "runs=" << runs << " timeout_ms=" << timeout_ms << '\n';
    std::cout << "elapsed_ms=" << elapsed_ms << '\n';
    std::cout << "total_root_vis=" << total_root_vis << '\n';
    std::cout << "vis_per_ms=" << (double(total_root_vis) / double(elapsed_ms)) << '\n';
    return 0;
}
