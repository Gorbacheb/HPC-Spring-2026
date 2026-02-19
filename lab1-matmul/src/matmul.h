#pragma once

void MatMulCPU(const float *A, const float *B, float *C, int n);

bool MatMulGPU(const float *A, const float *B, float *C, int n);
