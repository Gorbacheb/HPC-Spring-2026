#pragma once

void MatMulCPU(const float *A, const float *B, float *C, int n);

bool MatMulGPU(const float *A,
               const float *B,
               float *C,
               int n,
               float *kernel_ms,
               float *total_ms,
               int tile = 16);
