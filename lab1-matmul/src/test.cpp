#include <vector>

#include <catch2/catch_test_macros.hpp>

#include "matmul.h"
#include "random_utils.h"
#include "catch2/catch_approx.hpp"

TEST_CASE("MatMulCPU multiplies small matrices correctly") {
    const int n = 2;
    const std::vector<float> A = {
            1, 2,
            3, 4
    };
    const std::vector<float> B = {
            5, 6,
            7, 8
    };
    std::vector<float> C(n * n, 0.0f);

    MatMulCPU(A.data(), B.data(), C.data(), n);

    const std::vector<float> expected = {
            19.0f, 22.0f,
            43.0f, 50.0f
    };
    for (size_t i = 0; i < C.size(); ++i) {
        REQUIRE(C[i] == Catch::Approx(expected[i]).margin(1e-5f));
    }
}

TEST_CASE("MatMulCPU handles identity matrix") {
    const int n = 3;
    const std::vector<float> A = {
            1, 0, 0,
            0, 1, 0,
            0, 0, 1
    };
    const std::vector<float> B = {
            2, 3, 4,
            5, 6, 7,
            8, 9, 10
    };
    std::vector<float> C(n * n, 0.0f);

    MatMulCPU(A.data(), B.data(), C.data(), n);

    for (size_t i = 0; i < C.size(); ++i) {
        REQUIRE(C[i] == Catch::Approx(B[i]).margin(1e-5f));
    }
}

TEST_CASE("MatMulGPU matches CPU for random data") {
    const int sizes[] = {8, 17, 31};
    for (int n: sizes) {
        const size_t count = static_cast<size_t>(n) * n;
        std::vector<float> A(count);
        std::vector<float> B(count);
        std::vector<float> C_cpu(count);
        std::vector<float> C_gpu(count);

        FillRandom(A, 1234 + n);
        FillRandom(B, 5678 + n);

        MatMulCPU(A.data(), B.data(), C_cpu.data(), n);
        REQUIRE(MatMulGPU(A.data(), B.data(), C_gpu.data(), n));

        for (size_t i = 0; i < count; ++i) {
            REQUIRE(C_gpu[i] == Catch::Approx(C_cpu[i]).margin(1e-3f));
        }
    }
}
