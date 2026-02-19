#include <vector>

#include <catch2/catch_test_macros.hpp>

#include "matmul.h"
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
