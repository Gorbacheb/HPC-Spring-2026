#include <vector>

#include <catch2/catch_test_macros.hpp>

#include "vectorsum.h"
#include "catch2/catch_approx.hpp"
#include "../../common/random_utils.h"

TEST_CASE("SumCPU computes sum correctly for small array") {
    const std::vector<float> vec = {1.0f, 2.0f, 3.0f, 4.0f, 5.0f};
    float expected = 15.0f;
    float result = SumCpu(vec.data(), static_cast<int>(vec.size()));
    REQUIRE(result == Catch::Approx(expected).margin(1e-5f));
}

TEST_CASE("SumCPU handles edge cases") {
    const std::vector<float> vec1 = {42.0f};
    float result1 = SumCpu(vec1.data(), 1);
    REQUIRE(result1 == Catch::Approx(42.0f).margin(1e-5f));

    const std::vector<float> vec_empty;
    float result0 = SumCpu(vec_empty.data(), 0);
    REQUIRE(result0 == Catch::Approx(0.0f).margin(1e-5f));

    const std::vector<float> vec_neg = {-1.0f, -2.5f, 3.0f};
    float expected_neg = -0.5f;
    float result_neg = SumCpu(vec_neg.data(), static_cast<int>(vec_neg.size()));
    REQUIRE(result_neg == Catch::Approx(expected_neg).margin(1e-5f));
}

TEST_CASE("SumGPU matches CPU for random data") {
    const int sizes[] = {1, 2, 3, 7, 16, 31, 64, 127, 256, 511, 1024};
    for (int n: sizes) {
        auto vec = generate_random_vector(n, -10.0f, 10.0f, 12345 + n);

        float cpu_sum = SumCpu(vec.data(), n);
        float gpu_time = 0.0f;
        float gpu_sum = SumGpu(vec.data(), n, &gpu_time);

        INFO("n = " << n << ", cpu_sum = " << cpu_sum << ", gpu_sum = " << gpu_sum);
        REQUIRE(gpu_sum == Catch::Approx(cpu_sum).margin(1e-4f));
        REQUIRE(gpu_time > 0.0f);
    }
}

TEST_CASE("SumGPU accepts null time pointer") {
    const int n = 100;
    auto vec = generate_random_vector(n, -5.0f, 5.0f, 9999);
    float cpu_sum = SumCpu(vec.data(), n);
    float gpu_sum = SumGpu(vec.data(), n, nullptr);
    REQUIRE(gpu_sum == Catch::Approx(cpu_sum).margin(1e-4f));
}