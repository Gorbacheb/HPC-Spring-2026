#pragma once

float SumCpu(const float *vec, int n);

float SumGpu(const float *vec, int n, float *gpu_time_ms = nullptr);
