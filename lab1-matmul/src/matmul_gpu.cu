#include <cuda_runtime.h>
#include "matmul.h"
#include "cuda_utils.h"

constexpr int kTile = 16;

__global__ void MatMulKernel(const float *A, const float *B, float *C, int n) {
    const int row = blockIdx.y * kTile + threadIdx.y;
    const int col = blockIdx.x * kTile + threadIdx.x;
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

bool MatMulGPU(const float *A, const float *B, float *C, int n) {
    const size_t bytes = static_cast<size_t>(n) * n * sizeof(float);
    float *dA = nullptr;
    float *dB = nullptr;
    float *dC = nullptr;

    CUDA_CHECK_RET(cudaMalloc(&dA, bytes));
    CUDA_CHECK_RET(cudaMalloc(&dB, bytes));
    CUDA_CHECK_RET(cudaMalloc(&dC, bytes));

    CUDA_CHECK_RET(cudaMemcpy(dA, A, bytes, cudaMemcpyHostToDevice));
    CUDA_CHECK_RET(cudaMemcpy(dB, B, bytes, cudaMemcpyHostToDevice));

    const dim3 block(kTile, kTile);
    const dim3 grid((n + kTile - 1) / kTile, (n + kTile - 1) / kTile);
    MatMulKernel<<<grid, block>>>(dA, dB, dC, n);

    CUDA_CHECK_RET(cudaGetLastError());
    CUDA_CHECK_RET(cudaDeviceSynchronize());

    CUDA_CHECK_RET(cudaMemcpy(C, dC, bytes, cudaMemcpyDeviceToHost));

    cudaFree(dA);
    cudaFree(dB);
    cudaFree(dC);
    return true;
}
