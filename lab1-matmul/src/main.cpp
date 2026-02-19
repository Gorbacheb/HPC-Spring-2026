#include "matmul.h"

#include <chrono>
#include <iostream>
#include <string>
#include <vector>

#include "csv_writer.h"
#include "format_utils.h"
#include "math_utils.h"
#include "random_utils.h"

int main() {
    const std::vector<int> sizes = {128, 256, 512, 1024, 1 << 10, 1 << 11, 1 << 12, 1 << 13, 1 << 14};
    const std::vector<int> tiles = {8, 16, 32};
    const std::string csv_path = "../../lab1-matmul/lab1_matmul_results.csv";

    std::vector<std::vector<std::string>> csv_rows;
    csv_rows.push_back({"size", "tile", "cpu_ms", "gpu_kernel_ms", "gpu_total_ms", "speedup_kernel", "max_abs_error"});

    for (int n: sizes) {
        const size_t count = static_cast<size_t>(n) * n;
        std::vector<float> A(count);
        std::vector<float> B(count);
        std::vector<float> C_cpu(count);
        std::vector<float> C_gpu(count);

        FillRandom(A, 42u + n);
        FillRandom(B, 4242u + n);

        const bool run_cpu = n <= 2048;
        double cpu_ms = 0.0;
        if (run_cpu) {
            auto cpu_start = std::chrono::high_resolution_clock::now();
            MatMulCPU(A.data(), B.data(), C_cpu.data(), n);
            auto cpu_end = std::chrono::high_resolution_clock::now();
            cpu_ms = std::chrono::duration<double, std::milli>(cpu_end - cpu_start).count();
        }

        for (int tile: tiles) {
            float gpu_kernel_ms = 0.0f;
            float gpu_total_ms = 0.0f;
            if (!MatMulGPU(A.data(), B.data(), C_gpu.data(), n, &gpu_kernel_ms, &gpu_total_ms, tile)) {
                std::cerr << "GPU computation failed for N=" << n << " tile=" << tile << "\n";
                return 1;
            }

            const float max_diff = run_cpu ? MaxAbsDiff(C_cpu, C_gpu) : 0.0f;
            const double speedup = run_cpu ? (cpu_ms / static_cast<double>(gpu_kernel_ms)) : 0.0;

            std::cout << "N=" << n
                      << " TILE=" << tile
                      << " CPU=" << (run_cpu ? ToString(cpu_ms) : std::string("NA")) << " ms"
                      << " GPU(kernel)=" << ToString(gpu_kernel_ms) << " ms"
                      << " GPU(total)=" << ToString(gpu_total_ms) << " ms"
                      << " speedup=" << (run_cpu ? ToString(speedup) : std::string("NA"))
                      << " max_diff=" << (run_cpu ? ToString(max_diff) : std::string("NA"))
                      << "\n";

            csv_rows.push_back({
               std::to_string(n),
               std::to_string(tile),
               run_cpu ? ToString(cpu_ms) : "NA",
               ToString(gpu_kernel_ms),
               ToString(gpu_total_ms),
               run_cpu ? ToString(speedup) : "NA",
               run_cpu ? ToString(max_diff) : "NA"
            });
        }
    }

    if (!WriteCsv(csv_path, csv_rows)) {
        std::cerr << "Failed to write CSV: " << csv_path << "\n";
        return 1;
    }

    return 0;
}
