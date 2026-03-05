#include "state.hpp"

#include <cmath>
#include <functional>
#include <iostream>
#include <stdexcept>
#include <string>
#include <vector>

namespace {

void expect(bool condition, const std::string& message) {
    if (!condition) {
        throw std::runtime_error(message);
    }
}

bool almost_equal(float a, float b, float eps = 1e-6f) {
    return std::fabs(a - b) <= eps;
}

void naive_hurdles_step(uint32_t track, int pos, int move, int& out_pos, bool& hit) {
    int p = pos;
    if (move == 0) {
        p += 2;
    }
    else if (move == 1) {
        p += 1;
    }
    else if (move == 2) {
        p += 1;
        if (p <= TRACK_LENGTH + 1 && ((track >> p) & 1u) == 0) {
            p += 1;
        }
    }
    else if (move == 3) {
        p += 1;
        if (p <= TRACK_LENGTH + 1 && ((track >> p) & 1u) == 0) {
            p += 1;
            if (p <= TRACK_LENGTH + 1 && ((track >> p) & 1u) == 0) {
                p += 1;
            }
        }
    }

    if (p > TRACK_LENGTH + 1) {
        p = TRACK_LENGTH + 1;
    }

    out_pos = p;
    hit = ((track >> p) & 1u) != 0;
}

uint8_t identity_order() {
    uint8_t order = 0;
    order |= uint8_t(0) << (to_move_index('U') << 1);
    order |= uint8_t(1) << (to_move_index('L') << 1);
    order |= uint8_t(2) << (to_move_index('D') << 1);
    order |= uint8_t(3) << (to_move_index('R') << 1);
    return order;
}

void test_move_index_mapping() {
    expect(to_move_index('U') == 0, "to_move_index('U') must be 0");
    expect(to_move_index('L') == 1, "to_move_index('L') must be 1");
    expect(to_move_index('D') == 2, "to_move_index('D') must be 2");
    expect(to_move_index('R') == 3, "to_move_index('R') must be 3");
}

void test_hurdles_transitions() {
    Hurdles h;
    h.track = all_tracks[0];
    h.find_track_index();
    for (int i = 0; i < 3; i++) {
        h.pos[i] = 0;
        h.stun[i] = 0;
    }
    h.end = false;

    expect(h.greedy_moves(0) != 0, "Hurdles greedy mask must not be empty");

    Hurdles obstacle_hit;
    int obstacle_track_index = -1;
    int obstacle_start_pos = -1;
    for (int t = 0; t < tracks_count && obstacle_track_index < 0; ++t) {
        for (int p = 0; p < TRACK_LENGTH - 1; ++p) {
            if (Hurdles::next_hit[t][p][1] != 0) {
                obstacle_track_index = t;
                obstacle_start_pos = p;
                break;
            }
        }
    }
    expect(obstacle_track_index >= 0, "Expected at least one LEFT move to hit an obstacle");
    const uint8_t expected_obstacle_pos =
        Hurdles::next_pos[obstacle_track_index][obstacle_start_pos][1];
    obstacle_hit.track_index = obstacle_track_index;
    obstacle_hit.track = all_tracks[obstacle_track_index];
    obstacle_hit.end = false;
    for (int i = 0; i < 3; i++) {
        obstacle_hit.pos[i] = static_cast<int8_t>(obstacle_start_pos);
        obstacle_hit.stun[i] = 0;
    }

    const int8_t move_left[3] = {1, 1, 1};
    obstacle_hit.play(move_left);

    expect(obstacle_hit.pos[0] == expected_obstacle_pos, "Hurdles LEFT should use transition table");
    expect(obstacle_hit.stun[0] == 2, "Hurdles should apply stun after obstacle hit");

    Hurdles finish;
    finish.track_index = 0;
    finish.track = all_tracks[0];
    finish.end = false;
    for (int i = 0; i < 3; i++) {
        finish.pos[i] = 28;
        finish.stun[i] = 0;
    }

    const int8_t move_up[3] = {0, 0, 0};
    finish.play(move_up);
    expect(finish.end, "Hurdles should end when any runner reaches the finish line");
}

void test_hurdles_precomputed_step_table() {
    for (int track_id = 0; track_id < tracks_count; ++track_id) {
        const uint32_t track = all_tracks[track_id];
        for (int pos = 0; pos <= TRACK_LENGTH + 1; ++pos) {
            for (int move = 0; move < MOVE_COUNT; ++move) {
                int expected_pos = 0;
                bool expected_hit = false;
                naive_hurdles_step(track, pos, move, expected_pos, expected_hit);

                expect(
                    Hurdles::next_pos[track_id][pos][move] == expected_pos,
                    "Hurdles transition table next_pos mismatch"
                );
                expect(
                    Hurdles::next_hit[track_id][pos][move] == (expected_hit ? 1 : 0),
                    "Hurdles transition table next_hit mismatch"
                );
            }
        }
    }
}

void test_archery_transitions() {
    Archery a;
    a.end = false;
    a.wind_index = 0;
    a.wind[0] = 3;

    for (int i = 0; i < 3; i++) {
        a.x[i] = 0;
        a.y[i] = 0;
    }

    const int8_t moves[3] = {0, 1, 3}; // U, L, R
    a.play(moves);

    expect(a.x[0] == 0 && a.y[0] == -3, "Archery UP move must apply negative Y wind");
    expect(a.x[1] == -3 && a.y[1] == 0, "Archery LEFT move must apply negative X wind");
    expect(a.x[2] == 3 && a.y[2] == 0, "Archery RIGHT move must apply positive X wind");
    expect(a.end, "Archery should end when wind_index reaches zero");

    float places[3] = {0, 0, 0};
    a.generate_places(places);
    expect(almost_equal(places[0], 3.0f), "Archery tie at best score should award gold tier");
    expect(almost_equal(places[1], 3.0f), "Archery tie at best score should award gold tier");
    expect(almost_equal(places[2], 3.0f), "Archery tie at best score should award gold tier");
}

void test_skating_transitions() {
    Skating s;
    s.end = false;
    s.turns_left = 3;
    s.order = identity_order();

    s.dist_div10[0] = 0;
    s.dist_div10[1] = 0;
    s.dist_div10[2] = 0;

    s.dist_mod10[0] = 0;
    s.dist_mod10[1] = 8;
    s.dist_mod10[2] = 5;

    s.risk[0] = 0;
    s.risk[1] = 0;
    s.risk[2] = 0;

    const int8_t moves_a[3] = {3, 3, 3};
    s.play(moves_a);

    expect(!s.end, "Skating should still be active with turns_left > 1");
    expect(s.turns_left == 2, "Skating must decrement turns_left");
    expect(s.dist_div10[1] == 1 && s.dist_mod10[1] == 1, "Skating distance carry should work");
    expect(s.risk[0] == 2 && s.risk[1] == 2 && s.risk[2] == 2, "Skating risk update for rank-3 move must work");

    s.end = false;
    s.turns_left = 2;
    s.order = identity_order();

    s.risk[0] = 4;
    s.risk[1] = 0;
    s.risk[2] = 0;

    s.dist_div10[0] = 0;
    s.dist_div10[1] = 0;
    s.dist_div10[2] = 0;

    s.dist_mod10[0] = 0;
    s.dist_mod10[1] = 0;
    s.dist_mod10[2] = 1;

    const int8_t moves_b[3] = {2, 0, 0};
    s.play(moves_b);

    expect(s.risk[0] == -2, "Skating risk >= 5 must convert to two-turn stun");
}

void test_divings_transitions() {
    Divings d;
    d.end = false;
    d.goals_left = 2;
    d.goal = 1 | (2 << 2); // first: LEFT, second: DOWN

    for (int i = 0; i < 3; i++) {
        d.score[i] = 0;
        d.combo[i] = 0;
    }

    const int8_t first_moves[3] = {1, 0, 1};
    d.play(first_moves);

    expect(!d.end, "Divings should continue with goals_left > 1");
    expect(d.goals_left == 1, "Divings must decrement goals_left after a play");
    expect((d.goal & 3) == 2, "Divings should shift to the next goal");

    const int8_t second_moves[3] = {2, 2, 0};
    d.play(second_moves);

    expect(d.end, "Divings should end when last goal is consumed");
    expect(d.score[0] == 3, "Divings combo scoring should accumulate correctly");
    expect(d.score[1] == 1, "Divings scoring should update on a successful move");
    expect(d.score[2] == 1, "Divings failed combo should preserve prior score");

    float places[3] = {0, 0, 0};
    d.generate_places(places);
    expect(almost_equal(places[0], 3.0f), "Divings best score should get gold tier");
    expect(almost_equal(places[1], 1.0f), "Divings tied middle score should get silver tier");
    expect(almost_equal(places[2], 1.0f), "Divings tied middle score should get silver tier");

    Divings locked;
    locked.end = false;
    locked.goals_left = 1;
    locked.goal = 0;
    locked.score[0] = 100;
    locked.score[1] = 0;
    locked.score[2] = 0;
    locked.combo[0] = 0;
    locked.combo[1] = 0;
    locked.combo[2] = 0;

    expect(!locked.playable(0), "Divings playable() should detect inevitable first place");
}

void test_state_init_and_progression() {
    State s{};

    for (int i = 0; i < 3; i++) {
        s.hurdles_score[i] = 0.0f;
        s.archery_score[i] = 0.0f;
        s.skating_score[i] = 0.0f;
        s.divings_score[i] = 0.0f;
    }

    const std::vector<std::string> gpu = {
        ".......#...#...#...#...#......",
        "821141321141",
        "DRUL",
        "LLLDDUDLLDDU"
    };

    const std::vector<std::vector<int>> reg = {
        {2, 3, 1, 0, 0, 0, 0},
        {5, -5, 7, -7, 3, -7, 0},
        {2, 3, 2, 2, 2, 3, 14},
        {1, 0, 0, 1, 0, 0, 0}
    };

    s.init(gpu, reg);
    expect(!s.hurdles.end, "State::init should keep Hurdles active for non GAME_OVER input");
    expect(!s.archery.end, "State::init should keep Archery active for non GAME_OVER input");
    expect(!s.skating.end, "State::init should keep Skating active for non GAME_OVER input");
    expect(!s.divings.end, "State::init should keep Divings active for non GAME_OVER input");
    expect(s.archery.wind_index == 11, "State::init should parse wind length");
    expect(s.skating.turns_left == 5, "State::init should cap skating turns_left to 5");
    expect(s.divings.goals_left == 12, "State::init should parse diving goal sequence");

    s.turn = 0;
    s.play(0, 1, 2);
    expect(s.turn == 1, "State::play should increment global turn");
}

void test_state_rewards_sign() {
    State s{};

    s.hurdles.end = true;
    s.archery.end = true;
    s.skating.end = true;
    s.divings.end = true;

    s.hurdles_left = 0;
    s.archery_left = 0;
    s.skating_left = 0;
    s.divings_left = 0;

    s.hurdles.pos[0] = 29;
    s.hurdles.pos[1] = 20;
    s.hurdles.pos[2] = 10;

    s.archery.x[0] = 0;
    s.archery.y[0] = 0;
    s.archery.x[1] = 10;
    s.archery.y[1] = 0;
    s.archery.x[2] = 20;
    s.archery.y[2] = 0;

    s.skating.dist_div10[0] = 3;
    s.skating.dist_div10[1] = 2;
    s.skating.dist_div10[2] = 1;
    s.skating.dist_mod10[0] = 0;
    s.skating.dist_mod10[1] = 0;
    s.skating.dist_mod10[2] = 0;

    s.divings.score[0] = 30;
    s.divings.score[1] = 20;
    s.divings.score[2] = 10;

    for (int i = 0; i < 3; i++) {
        s.hurdles_score[i] = 0.0f;
        s.archery_score[i] = 0.0f;
        s.skating_score[i] = 0.0f;
        s.divings_score[i] = 0.0f;
    }

    float rewards[3] = {0, 0, 0};
    s.get_stats(rewards);

    expect(rewards[0] > 0.0f, "Dominating player reward should be positive");
    expect(rewards[1] < 0.0f, "Losing player reward should be negative");
    expect(rewards[2] < 0.0f, "Losing player reward should be negative");
}

} // namespace

int main() {
    fast_srand(2137420);
    Hurdles::build_dp();

    const std::vector<std::pair<std::string, std::function<void()>>> tests = {
        {"move index mapping", test_move_index_mapping},
        {"hurdles transitions", test_hurdles_transitions},
        {"hurdles transition table", test_hurdles_precomputed_step_table},
        {"archery transitions", test_archery_transitions},
        {"skating transitions", test_skating_transitions},
        {"divings transitions", test_divings_transitions},
        {"state init and progression", test_state_init_and_progression},
        {"state rewards sign", test_state_rewards_sign},
    };

    int passed = 0;

    for (const auto& [name, fn] : tests) {
        try {
            fn();
            ++passed;
            std::cout << "[PASS] " << name << '\n';
        }
        catch (const std::exception& ex) {
            std::cerr << "[FAIL] " << name << ": " << ex.what() << '\n';
            return 1;
        }
    }

    std::cout << "All tests passed: " << passed << '\n';
    return 0;
}
