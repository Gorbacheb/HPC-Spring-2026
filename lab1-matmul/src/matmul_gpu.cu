#include <chrono>
#include <cuda_runtime.h>

#include "cuda_utils.h"
#include "matmul.h"

__global__ void MatMulKernelGlobal(const float *A, const float *B, float *C, int n) {
    const int row = blockIdx.y * blockDim.y + threadIdx.y;
    const int col = blockIdx.x * blockDim.x + threadIdx.x;
    if (row >= n || col >= n) {
        return;
    }

    float sum = 0.0f;
    const int row_offset = row * n;
    for (int k = 0; k < n; ++k) {
        sum += A[row_offset + k] * B[k * n + col];
    }
    C[row_offset + col] = sum;
}

bool MatMulGPU(const float *A,
               const float *B,
               float *C,
               int n,
               float *kernel_ms,
               float *total_ms,
               int tile) {
    const auto total_start = std::chrono::high_resolution_clock::now();

    const size_t bytes = static_cast<size_t>(n) * n * sizeof(float);
    float *dA = nullptr;
    float *dB = nullptr;
    float *dC = nullptr;

    CUDA_CHECK_RET(cudaMalloc(&dA, bytes));
    CUDA_CHECK_RET(cudaMalloc(&dB, bytes));
    CUDA_CHECK_RET(cudaMalloc(&dC, bytes));

    CUDA_CHECK_RET(cudaMemcpy(dA, A, bytes, cudaMemcpyHostToDevice));
    CUDA_CHECK_RET(cudaMemcpy(dB, B, bytes, cudaMemcpyHostToDevice));

    cudaEvent_t start = nullptr;
    cudaEvent_t stop = nullptr;
    CUDA_CHECK_RET(cudaEventCreate(&start));
    CUDA_CHECK_RET(cudaEventCreate(&stop));

    const dim3 block(tile, tile);
    const dim3 grid((n + tile - 1) / tile, (n + tile - 1) / tile);

    CUDA_CHECK_RET(cudaEventRecord(start));
    MatMulKernelGlobal<<<grid, block>>>(dA, dB, dC, n);
    CUDA_CHECK_RET(cudaEventRecord(stop));
    CUDA_CHECK_RET(cudaEventSynchronize(stop));
    CUDA_CHECK_RET(cudaGetLastError());

    float elapsed_ms = 0.0f;
    CUDA_CHECK_RET(cudaEventElapsedTime(&elapsed_ms, start, stop));
    if (kernel_ms) {
        *kernel_ms = elapsed_ms;
    }

    CUDA_CHECK_RET(cudaMemcpy(C, dC, bytes, cudaMemcpyDeviceToHost));

    CUDA_CHECK_RET(cudaEventDestroy(start));
    CUDA_CHECK_RET(cudaEventDestroy(stop));
    CUDA_CHECK_RET(cudaFree(dA));
    CUDA_CHECK_RET(cudaFree(dB));
    CUDA_CHECK_RET(cudaFree(dC));

    const auto total_end = std::chrono::high_resolution_clock::now();
    if (total_ms) {
        *total_ms = std::chrono::duration<double, std::milli>(total_end - total_start).count();
    }

    return true;
}
