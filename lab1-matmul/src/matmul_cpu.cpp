#include "matmul.h"

void MatMulCPU(const float *A, const float *B, float *C, int n) {
    for (int row = 0; row < n; ++row) {
        for (int col = 0; col < n; ++col) {
            float sum = 0.0f;
            const int row_offset = row * n;
            for (int k = 0; k < n; ++k) {
                sum += A[row_offset + k] * B[k * n + col];
            }
            C[row_offset + col] = sum;
        }
    }
}
