#pragma once

#include <cmath>
#include <vector>

inline float MaxAbsDiff(const std::vector<float> &a, const std::vector<float> &b) {
    float max_diff = 0.0f;
    const size_t count = a.size();
    for (size_t i = 0; i < count; ++i) {
        const float diff = std::fabs(a[i] - b[i]);
        if (diff > max_diff) {
            max_diff = diff;
        }
    }
    return max_diff;
}
