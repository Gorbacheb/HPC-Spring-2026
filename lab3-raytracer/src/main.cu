#include <exception>
#include <cctype>
#include <cstdlib>
#include <filesystem>
#include <iostream>
#include <optional>
#include <string>

#include "camera.h"
#include "cuda_renderer.cuh"
#include "random_scene.h"
#include "render_options.h"
#include "scene_io.h"

namespace {

bool HasObjExtension(const std::filesystem::path& path) {
    std::string extension = path.extension().string();
    for (char& ch : extension) {
        ch = static_cast<char>(std::tolower(static_cast<unsigned char>(ch)));
    }
    return extension == ".obj";
}

bool TryParseInt(const std::string& text, int* value) {
    char* end = nullptr;
    const long parsed = std::strtol(text.c_str(), &end, 10);
    if (end == text.c_str() || *end != '\0') {
        return false;
    }
    *value = static_cast<int>(parsed);
    return true;
}

void PrintUsage(const char* program) {
    std::cerr << "Usage: " << program
              << " [scene.obj] [output.bmp] [width] [height] [depth]\n"
              << "If scene.obj is omitted, a random scene is generated with curand.\n";
}

}  // namespace

int main(int argc, char** argv) {
    if (argc < 1) {
        PrintUsage(argv[0]);
        return 1;
    }

    try {
        int arg_index = 1;
        std::optional<std::filesystem::path> scene_path;
        if (arg_index < argc && HasObjExtension(argv[arg_index])) {
            scene_path = std::filesystem::path(argv[arg_index]);
            ++arg_index;
        }

        std::string output_path = "lab3_raytracer.bmp";
        if (arg_index < argc) {
            int dummy = 0;
            if (!TryParseInt(argv[arg_index], &dummy)) {
                output_path = argv[arg_index];
                ++arg_index;
            }
        }

        CameraOptions camera_options;
        if (arg_index < argc) {
            camera_options.screen_width = std::stoi(argv[arg_index]);
            ++arg_index;
        }
        if (arg_index < argc) {
            camera_options.screen_height = std::stoi(argv[arg_index]);
            ++arg_index;
        }

        RenderOptions render_options;
        if (arg_index < argc) {
            render_options.depth = std::stoi(argv[arg_index]);
        }

        const Scene scene = scene_path.has_value() ? ReadScene(*scene_path) : GenerateRandomScene();
        const Camera camera(camera_options);

        auto device_scene = scene.UploadScene();
        RenderSceneToBmp(camera, device_scene, render_options.depth, output_path);
        device_scene.Release();
        return 0;
    } catch (const std::exception& e) {
        std::cerr << "Error: " << e.what() << '\n';
        return 1;
    }
}
