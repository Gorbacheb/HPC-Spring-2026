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
    const std::vector<int> sizes = {128, 256, 512, 1024, 1536, 1 << 11, 1 << 12, 1 << 13, 1 << 14};
    const std::vector<int> tiles = {8, 16, 32};
    const std::string csv_path = "../../lab1-matmul/lab1_matmul_results.csv";

    std::vector<std::vector<std::string>> csv_rows;
    csv_rows.push_back({
        "size",
        "tile",
        "cpu_ms",
        "gpu_shared_kernel_ms",
        "gpu_shared_total_ms",
        "gpu_global_kernel_ms",
        "gpu_global_total_ms",
        "speedup_shared_kernel",
        "speedup_global_kernel",
        "max_abs_error_shared",
        "max_abs_error_global"
    });

    for (int n: sizes) {
        const size_t count = static_cast<size_t>(n) * n;
        std::vector<float> A(count);
        std::vector<float> B(count);
        std::vector<float> C_cpu(count);
        std::vector<float> C_gpu(count);

        FillRandom(A, 42u + n);
        FillRandom(B, 4242u + n);

        const bool run_cpu = n <= 1536;
        double cpu_ms = 0.0;
        if (run_cpu) {
            auto cpu_start = std::chrono::high_resolution_clock::now();
            MatMulCPU(A.data(), B.data(), C_cpu.data(), n);
            auto cpu_end = std::chrono::high_resolution_clock::now();
            cpu_ms = std::chrono::duration<double, std::milli>(cpu_end - cpu_start).count();
        }

        for (int tile: tiles) {
            float shared_kernel_ms = 0.0f;
            float shared_total_ms = 0.0f;
            if (!MatMulGPU(A.data(), B.data(), C_gpu.data(), n, &shared_kernel_ms, &shared_total_ms, tile)) {
                std::cerr << "GPU shared computation failed for N=" << n << " tile=" << tile << "\n";
                return 1;
            }

            float max_diff_shared;
            double speedup_shared;
            if (run_cpu) {
                max_diff_shared = MaxAbsDiff(C_cpu, C_gpu);
                speedup_shared = cpu_ms / static_cast<double>(shared_kernel_ms);
            }

            float global_kernel_ms = 0.0f;
            float global_total_ms = 0.0f;
            if (!MatMulGPU_Global(A.data(), B.data(), C_gpu.data(), n, &global_kernel_ms, &global_total_ms, tile)) {
                std::cerr << "GPU global computation failed for N=" << n << " tile=" << tile << "\n";
                return 1;
            }

            float max_diff_global;
            double speedup_global;
            if (run_cpu) {
                max_diff_global = MaxAbsDiff(C_cpu, C_gpu);
                speedup_global = cpu_ms / static_cast<double>(global_kernel_ms);
            }

            std::cout << "N=" << n
                      << " TILE=" << tile
                      << " CPU=" << (run_cpu ? ToString(cpu_ms) : std::string("NA")) << " ms"
                      << " GPU(shared)=" << ToString(shared_kernel_ms) << " ms"
                      << " GPU(global)=" << ToString(global_kernel_ms) << " ms"
                      << " speedup_shared=" << (run_cpu ? ToString(speedup_shared) : std::string("NA"))
                      << " speedup_global=" << (run_cpu ? ToString(speedup_global) : std::string("NA"))
                      << " max_diff_shared=" << (run_cpu ? ToString(max_diff_shared) : std::string("NA"))
                      << " max_diff_global=" << (run_cpu ? ToString(max_diff_global) : std::string("NA"))
                      << "\n";

            csv_rows.push_back({
                std::to_string(n),
                std::to_string(tile),
                run_cpu ? ToString(cpu_ms) : "NA",
                ToString(shared_kernel_ms),
                ToString(shared_total_ms),
                ToString(global_kernel_ms),
                ToString(global_total_ms),
                run_cpu ? ToString(speedup_shared) : "NA",
                run_cpu ? ToString(speedup_global) : "NA",
                run_cpu ? ToString(max_diff_shared) : "NA",
                run_cpu ? ToString(max_diff_global) : "NA"
            });
        }
    }

    if (!WriteCsv(csv_path, csv_rows)) {
        std::cerr << "Failed to write CSV: " << csv_path << "\n";
        return 1;
    }

    return 0;
}
