#pragma once

#include <cuda_runtime.h>

#include <cstdio>
#include <cstdlib>

inline bool CudaCheck(cudaError_t err, const char *file, int line, bool abort) {
    if (err != cudaSuccess) {
        std::fprintf(stderr, "CUDA error %s:%d: %s\n", file, line, cudaGetErrorString(err));
        if (abort) {
            std::exit(1);
        }
        return false;
    }
    return true;
}

#define CUDA_CHECK(call) (CudaCheck((call), __FILE__, __LINE__, true))

#define CUDA_CHECK_RET(call)                 \
    do {                                     \
        if (!CudaCheck((call), __FILE__, __LINE__, false)) { \
            return false;                    \
        }                                    \
    } while (0)
