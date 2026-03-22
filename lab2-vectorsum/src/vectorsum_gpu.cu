#include <cuda_runtime.h>
#include <vector>
#include "vectorsum.h"
#include "cuda_utils.h"

template<int ITEMS_PER_THREAD>
__global__ void SumKernelOpt(const float *in, float *out, int n) {
    extern __shared__ float sdata[];

    unsigned int tid = threadIdx.x;
    unsigned int idx = blockIdx.x * blockDim.x * ITEMS_PER_THREAD + tid;

    float sum = 0.0f;

#pragma unroll
    for (int i = 0; i < ITEMS_PER_THREAD; ++i) {
        unsigned int current_idx = idx + i * blockDim.x;
        if (current_idx < n) {
            sum += in[current_idx];
        }
    }

    sdata[tid] = sum;
    __syncthreads();

    for (unsigned int s = blockDim.x / 2; s > 0; s >>= 1) {
        if (tid < s) {
            sdata[tid] += sdata[tid + s];
        }
        __syncthreads();
    }

    if (tid == 0) {
        out[blockIdx.x] = sdata[0];
    }
}

float SumGpu(const float *h_in, int n, float *gpu_time_ms) {
    const int ITEMS_PER_THREAD = 16;
    const int threadsPerBlock = 512;
    int blocks = (n + threadsPerBlock * ITEMS_PER_THREAD - 1) / (threadsPerBlock * ITEMS_PER_THREAD);
    size_t in_bytes = n * sizeof(float);
    size_t out_bytes = blocks * sizeof(float);

    float *d_in = nullptr, *d_out = nullptr;
    CUDA_CHECK(cudaMalloc(&d_in, in_bytes));
    CUDA_CHECK(cudaMalloc(&d_out, out_bytes));

    CUDA_CHECK(cudaMemcpy(d_in, h_in, in_bytes, cudaMemcpyHostToDevice));

    SumKernelOpt<ITEMS_PER_THREAD><<<blocks, threadsPerBlock, threadsPerBlock * sizeof(float)>>>(d_in, d_out, n);
    CUDA_CHECK(cudaDeviceSynchronize());

    cudaEvent_t start, stop;
    float kernel_time_ms = 0.0f;
    if (gpu_time_ms) {
        CUDA_CHECK(cudaEventCreate(&start));
        CUDA_CHECK(cudaEventCreate(&stop));
        CUDA_CHECK(cudaEventRecord(start, nullptr));
    }

    SumKernelOpt<ITEMS_PER_THREAD><<<blocks, threadsPerBlock, threadsPerBlock * sizeof(float)>>>(d_in, d_out, n);
    CUDA_CHECK(cudaGetLastError());
    CUDA_CHECK(cudaDeviceSynchronize());

    if (gpu_time_ms) {
        CUDA_CHECK(cudaEventRecord(stop, nullptr));
        CUDA_CHECK(cudaEventSynchronize(stop));
        CUDA_CHECK(cudaEventElapsedTime(&kernel_time_ms, start, stop));
        CUDA_CHECK(cudaEventDestroy(start));
        CUDA_CHECK(cudaEventDestroy(stop));
        *gpu_time_ms = kernel_time_ms;
    }

    std::vector<float> h_out(blocks);
    CUDA_CHECK(cudaMemcpy(h_out.data(), d_out, out_bytes, cudaMemcpyDeviceToHost));

    CUDA_CHECK(cudaFree(d_in));
    CUDA_CHECK(cudaFree(d_out));

    float total = 0.0f;
    for (float v: h_out) {
        total += v;
    }

    return total;
}