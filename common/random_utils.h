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

std::vector<float> generate_random_vector(size_t n, float min, float max, unsigned seed) {
    static std::mt19937 gen(seed ? seed : std::random_device{}());
    if (seed != 0) {
        gen.seed(seed);
    }
    std::uniform_real_distribution<float> dist(min, max);
    std::vector<float> vec(n);
    for (size_t i = 0; i < n; ++i) {
        vec[i] = dist(gen);
    }
    return vec;
}
