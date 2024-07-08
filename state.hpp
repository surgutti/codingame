#ifndef STATE_HPP
#define STATE_HPP

#include "const.hpp"
#include "unit.hpp"

#include <iostream>

struct State {

    Unit pods[PODS_NB];

    int laps;
    int checkpoints_count;

    Unit checkpoints[CHECKPOINTS_NB];

    void tick() {
        float t = 0.0f;

        while (t < 1.0f) {
            Unit *p, *q;
            float collision_time = 1.0f - t;

            for (int i = 0; i < PODS_NB; i++) {
                for (int j = i + 1; j < PODS_NB; j++) {
                    float new_collision_time;

                    if (predict_collision(pods[i], pods[j], &new_collision_time)) {
                        if (collision_time > new_collision_time) {
                            collision_time = new_collision_time;
                            p = &pods[i];
                            q = &pods[j];
                        }
                    }
                }
            }

            for (int i = 0; i < PODS_NB; i++) {
                if (is_checkpoint_completed(pods[i], checkpoints[pods[i].cp_next], collision_time)) {
                    if (++pods[i].cp_next == checkpoints_count) {
                        pods[i].cp_next = 0;
                    }
                    pods[i].cp_pass++;
                }

                pods[i].move(collision_time);
            }

            t += collision_time;

            if (t == 1.0f) {
                break;
            }

            simulate_bounce(p, q);
        }

        for (int i = 0; i < PODS_NB; i++) {
            pods[i].end_turn();
        }
    }

    void init() {
        std::cin >> laps; std::cin.ignore();
        std::cin >> checkpoints_count; std::cin.ignore();

        for (int i = 0; i < checkpoints_count; i++) {
            std::cin >> checkpoints[i].x >> checkpoints[i].y; std::cin.ignore();
        }
    }

    void read() {
        for (int i = 0; i < PODS_NB; i++) {
            int cp_next;

            std::cin >> pods[i].x >>
                        pods[i].y >>
                        pods[i].vx >>
                        pods[i].vy >>
                        pods[i].angle >>
                        cp_next;
            std::cin.ignore();

            if (cp_next != pods[i].cp_next) {
                pods[i].cp_next = cp_next;
                pods[i].cp_pass++;
            }

            pods[i].angle *= PI / 180.0f;
        }
    }

    void debug() const {
        for (int i = 0; i < PODS_NB; i++) {
            std::cerr << "id: " << i << '\n';
            std::cerr << "x : " << pods[i].x << '\n';
            std::cerr << "y : " << pods[i].y << '\n';
            std::cerr << "vx: " << pods[i].vx << '\n';
            std::cerr << "vy: " << pods[i].vy << '\n';
            std::cerr << "nx: " << pods[i].cp_next << '\n';
            std::cerr << "pa: " << pods[i].cp_pass << '\n';
            std::cerr << "an: " << pods[i].angle << '\n';
            std::cerr << "b : " << pods[i].boosted << '\n';
            std::cerr << "----\n";
        }
    }
};

#endif // STATE_HPP