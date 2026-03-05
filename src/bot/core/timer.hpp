#ifndef TIMER_HPP
#define TIMER_HPP

#include <chrono>

class Timer {
public:

    void start() {
        start_time = std::chrono::steady_clock::now();
    }

    inline auto get_elapsed() {
        using namespace std::chrono;
        milliseconds elapsed = duration_cast<milliseconds>(steady_clock::now() - start_time);
        return elapsed.count();
    }

private:
    std::chrono::steady_clock::time_point start_time;
} timer;

#endif
