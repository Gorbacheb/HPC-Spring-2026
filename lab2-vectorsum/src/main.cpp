#include <iostream>
#include <iomanip>
#include <chrono>
#include <cmath>
#include <sstream>
#include "vectorsum.h"
#include "random_utils.h"
#include "csv_writer.h"

int main() {
    const std::vector<int> sizes = {128, 256, 512, 1024, 1536, 1 << 11, 1 << 12, 1 << 13, 1 << 14, 1 << 15, 1 << 16,
                                    1 << 17, 1 << 18, 1 << 19, 1 << 20};
    const std::string csv_path = "../../lab2-vectorsum/vectorsum.csv";

    std::vector<std::vector<std::string>> csv_rows;
    csv_rows.push_back({
                               "size",
                               "cpu_ms",
                               "gpu_ms",
                               "speedup",
                               "check",
                       });
    std::cout << std::fixed << std::setprecision(3);
    std::cout << "|   N    |  CPU time (ms) |  GPU time (ms) |  Speedup  |  Check  |\n";

    for (int n: sizes) {
        auto vec = generate_random_vector(n, 0.0f, 1.0f, 12345);

        auto cpu_start = std::chrono::high_resolution_clock::now();
        float sum_cpu_result = SumCpu(vec.data(), n);
        auto cpu_end = std::chrono::high_resolution_clock::now();
        double cpu_time = std::chrono::duration<double, std::milli>(cpu_end - cpu_start).count();

        float gpu_time = 0.0f;
        float sum_gpu_result = SumGpu(vec.data(), n, &gpu_time);

        double speedup = cpu_time / gpu_time;

        float diff = std::fabs(sum_cpu_result - sum_gpu_result);
        float rel_diff = diff / (std::fabs(sum_cpu_result) + 1e-6f);
        bool ok = rel_diff < 1e-5f;

        std::cout << "| " << std::setw(7) << n << " | "
                  << std::setw(13) << cpu_time << " | "
                  << std::setw(13) << gpu_time << " | "
                  << std::setw(9) << speedup << " |  "
                  << (ok ? "OK" : "FAIL") << "   |\n";

        std::ostringstream cpu_ss, gpu_ss, speedup_ss;
        cpu_ss << std::fixed << std::setprecision(3) << cpu_time;
        gpu_ss << std::fixed << std::setprecision(3) << gpu_time;
        speedup_ss << std::fixed << std::setprecision(3) << speedup;

        csv_rows.push_back({
                                   std::to_string(n),
                                   cpu_ss.str(),
                                   gpu_ss.str(),
                                   speedup_ss.str(),
                                   ok ? "OK" : "FAIL"
                           });
    }

    if (!WriteCsv(csv_path, csv_rows)) {
        std::cerr << "Failed to write CSV: " << csv_path << "\n";
        return 1;
    }

    return 0;
}