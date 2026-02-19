#pragma once

#include <random>
#include <vector>

inline void FillRandom(std::vector<float>& v, uint32_t seed, float lo = -1.0f, float hi = 1.0f) {
    std::mt19937 rng(seed);
    std::uniform_real_distribution<float> dist(lo, hi);
    for (float& x : v) {
        x = dist(rng);
    }
}
