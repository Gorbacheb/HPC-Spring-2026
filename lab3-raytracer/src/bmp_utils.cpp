#include "bmp_utils.h"

#include <algorithm>
#include <cmath>
#include <cstdlib>
#include <iostream>
#include <vector>

#define STB_IMAGE_WRITE_STATIC
#define STB_IMAGE_WRITE_IMPLEMENTATION
#include <stb_image_write.h>

void SaveBmp(const std::string& path, const std::vector<Vector>& pixels, int width, int height) {
    std::vector<unsigned char> image(static_cast<size_t>(width) * static_cast<size_t>(height) * 3);
    double tone_map_scale = 0.0;
    for (const Vector& pixel : pixels) {
        tone_map_scale = std::max(tone_map_scale, std::max(pixel[0], std::max(pixel[1], pixel[2])));
    }
    if (tone_map_scale < 1e-12) {
        tone_map_scale = 1.0;
    }

    const auto clamp01 = [](double value) { return std::max(0.0, std::min(1.0, value)); };

    const auto tone_map = [&](double value) {
        const double c2 = tone_map_scale * tone_map_scale;
        return (value * (1.0 + value / c2)) / (1.0 + value);
    };

    const auto clamp_byte = [](double value) {
        value = std::max(0.0, std::min(1.0, value));
        return static_cast<unsigned char>(value * 255.0 + 0.5);
    };

    for (int y = 0; y < height; ++y) {
        for (int x = 0; x < width; ++x) {
            const size_t src_idx =
                static_cast<size_t>(y) * static_cast<size_t>(width) + static_cast<size_t>(x);
            const size_t dst_idx = src_idx * 3;
            const Vector c = pixels[src_idx];
            const double r = std::pow(clamp01(tone_map(c[0])), 1.0 / 2.2);
            const double g = std::pow(clamp01(tone_map(c[1])), 1.0 / 2.2);
            const double b = std::pow(clamp01(tone_map(c[2])), 1.0 / 2.2);
            image[dst_idx + 0] = clamp_byte(r);
            image[dst_idx + 1] = clamp_byte(g);
            image[dst_idx + 2] = clamp_byte(b);
        }
    }

    if (stbi_write_bmp(path.c_str(), width, height, 3, image.data()) == 0) {
        std::cerr << "Failed to write BMP file: " << path << '\n';
        std::exit(EXIT_FAILURE);
    }
}
