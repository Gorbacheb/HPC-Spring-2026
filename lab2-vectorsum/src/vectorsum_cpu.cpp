#include "vectorsum.h"

float SumCpu(const float *vec, int n) {
    double s = 0.0;
    for (int i = 0; i < n; ++i) {
        s += vec[i];
    }
    return static_cast<float>(s);
}