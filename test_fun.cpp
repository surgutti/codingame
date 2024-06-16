#include "utils.hpp"

#include <iostream>
#include <cassert>
#include <cmath>
#include <random>

int main() {

    std::mt19937 rng(2137);

    auto randfloat = [&](double a, double b) {
        return std::uniform_real_distribution<double>(a, b)(rng);
    };

    long double error = 0;
    for (int rep = 0; rep < 10000; rep++) {
        float x = randfloat(1, 100);

        float ans = 1.0 / std::sqrt(x);
        float res = rsqrt_fast(x);

        error = std::max<long double>(error, fabs(ans - res));

        std::cerr << "ERROR: " << error << '\n';
    }
}