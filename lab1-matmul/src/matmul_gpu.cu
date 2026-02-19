#include <chrono>
#include <cuda_runtime.h>

#include "cuda_utils.h"
#include "matmul.h"

//не придумал как избавиться от шаблона, возможно можно сделать красиво через constexpr
template<int TILE>
__global__ void MatMulKernelShared(const float *A, const float *B, float *C, int n) {
    __shared__ float As[TILE][TILE];
    __shared__ float Bs[TILE][TILE];

    const int row = blockIdx.y * TILE + threadIdx.y;
    const int col = blockIdx.x * TILE + threadIdx.x;

    float sum = 0;
    const int tiles = (n + TILE - 1) / TILE;
    for (int t = 0; t < tiles; ++t) {
        const int a_col = t * TILE + threadIdx.x;
        const int b_row = t * TILE + threadIdx.y;

        As[threadIdx.y][threadIdx.x] = (row < n && a_col < n) ? A[row * n + a_col] : 0;
        Bs[threadIdx.y][threadIdx.x] = (b_row < n && col < n) ? B[b_row * n + col] : 0;
        __syncthreads();

        for (int k = 0; k < TILE; ++k) {
            sum += As[threadIdx.y][k] * Bs[k][threadIdx.x];
        }
        __syncthreads();
    }

    if (row < n && col < n) {
        C[row * n + col] = sum;
    }
}

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

//костыли для шаблона
void LaunchKernelShared(const float *A, const float *B, float *C, int n, int tile) {
    const dim3 block(tile, tile);
    const dim3 grid((n + tile - 1) / tile, (n + tile - 1) / tile);
    switch (tile) {
        case 8:
            MatMulKernelShared<8><<<grid, block>>>(A, B, C, n);
            break;
        case 16:
            MatMulKernelShared<16><<<grid, block>>>(A, B, C, n);
            break;
        case 32:
            MatMulKernelShared<32><<<grid, block>>>(A, B, C, n);
            break;
        default:
            break;
    }
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

    CUDA_CHECK_RET(cudaEventRecord(start));
    LaunchKernelShared(dA, dB, dC, n, tile);
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

bool MatMulGPU_Global(const float *A,
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
