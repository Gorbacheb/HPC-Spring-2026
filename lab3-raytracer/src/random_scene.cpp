#include "random_scene.h"

#include <curand.h>

#include <stdexcept>
#include <string>
#include <vector>
#include <random>

namespace {

void CheckCurand(curandStatus_t status, const char* what) {
    if (status != CURAND_STATUS_SUCCESS) {
        throw std::runtime_error(std::string(what) + ": curand status code " +
                                 std::to_string(static_cast<int>(status)));
    }
}

class CurandGenerator {
public:
    explicit CurandGenerator(uint64_t seed) {
        CheckCurand(curandCreateGenerator(&generator_, CURAND_RNG_PSEUDO_DEFAULT),
                    "curandCreateGenerator");
        CheckCurand(curandSetPseudoRandomGeneratorSeed(generator_, seed),
                    "curandSetPseudoRandomGeneratorSeed");
    }

    CurandGenerator(const CurandGenerator&) = delete;
    CurandGenerator& operator=(const CurandGenerator&) = delete;

    ~CurandGenerator() {
        if (generator_ != nullptr) {
            curandDestroyGenerator(generator_);
        }
    }

    [[nodiscard]] curandGenerator_t Get() const {
        return generator_;
    }

private:
    curandGenerator_t generator_ = nullptr;
};

class DeviceDoubleBuffer {
public:
    explicit DeviceDoubleBuffer(size_t count) {
        if (count > 0U) {
            CUDA_CHECK(cudaMalloc(&data_, count * sizeof(double)));
        }
    }

    DeviceDoubleBuffer(const DeviceDoubleBuffer&) = delete;
    DeviceDoubleBuffer& operator=(const DeviceDoubleBuffer&) = delete;

    ~DeviceDoubleBuffer() {
        if (data_ != nullptr) {
            cudaFree(data_);
        }
    }

    [[nodiscard]] double* Get() const {
        return data_;
    }

private:
    double* data_ = nullptr;
};

std::vector<double> GenerateUniformSamples(size_t count, uint64_t seed) {
    CurandGenerator generator(seed);
    DeviceDoubleBuffer device_buffer(count);
    CheckCurand(curandGenerateUniformDouble(generator.Get(), device_buffer.Get(), count),
                "curandGenerateUniformDouble");

    std::vector<double> samples(count);
    CUDA_CHECK(cudaMemcpy(samples.data(), device_buffer.Get(), count * sizeof(double),
                          cudaMemcpyDeviceToHost));
    return samples;
}

double NextSample(const std::vector<double>& samples, size_t* cursor) {
    const double value = samples[*cursor % samples.size()];
    ++(*cursor);
    return value;
}

double SampleRange(const std::vector<double>& samples, size_t* cursor, double lo, double hi) {
    return lo + (hi - lo) * NextSample(samples, cursor);
}

Vector SampleVector(const std::vector<double>& samples, size_t* cursor, double lo, double hi) {
    return {SampleRange(samples, cursor, lo, hi), SampleRange(samples, cursor, lo, hi),
            SampleRange(samples, cursor, lo, hi)};
}

Material MakeMaterial(const std::vector<double>& samples, size_t* cursor) {
    Material material;
    material.ambient_color = SampleVector(samples, cursor, 0.01, 0.08);
    material.diffuse_color = SampleVector(samples, cursor, 0.2, 0.95);
    material.specular_color = SampleVector(samples, cursor, 0.05, 0.45);
    material.intensity = Vector(0.0, 0.0, 0.0);
    material.albedo =
        Vector(SampleRange(samples, cursor, 0.45, 0.85), SampleRange(samples, cursor, 0.05, 0.35),
               SampleRange(samples, cursor, 0.0, 0.2));
    material.specular_exponent = SampleRange(samples, cursor, 8.0, 80.0);
    material.refraction_index = 1.0;
    return material;
}

}  // namespace

Scene GenerateRandomScene() {
    auto kSeed = std::random_device{}();
    const std::vector<double> samples = GenerateUniformSamples(1024U, kSeed);
    size_t cursor = 0U;

    Scene scene;

    Material ground;
    ground.ambient_color = Vector(0.04, 0.04, 0.04);
    ground.diffuse_color = Vector(0.45, 0.42, 0.4);
    ground.specular_color = Vector(0.12, 0.12, 0.12);
    ground.albedo = Vector(0.9, 0.08, 0.0);
    ground.specular_exponent = 24.0;
    const int ground_material_id = scene.AddMaterial(ground);
    scene.AddSphere(Sphere(Vector(0.0, -1000.5, -6.0), 1000.0), ground_material_id);

    Material center;
    center.ambient_color = Vector(0.03, 0.03, 0.05);
    center.diffuse_color = SampleVector(samples, &cursor, 0.25, 0.9);
    center.specular_color = Vector(0.35, 0.35, 0.35);
    center.albedo = Vector(0.2, 0.65, 0.15);
    center.specular_exponent = 96.0;
    center.refraction_index = 1.45;
    const int center_material_id = scene.AddMaterial(center);
    scene.AddSphere(Sphere(Vector(0.0, 0.75, -5.0), 0.75), center_material_id);

    Material glass;
    glass.ambient_color = Vector(0.01, 0.01, 0.02);
    glass.diffuse_color = Vector(0.18, 0.2, 0.25);
    glass.specular_color = Vector(0.85, 0.85, 0.9);
    glass.albedo = Vector(0.05, 0.1, 0.85);
    glass.specular_exponent = 128.0;
    glass.refraction_index = 1.5;
    const int glass_material_id = scene.AddMaterial(glass);
    scene.AddSphere(Sphere(Vector(-1.8, 0.55, -4.4), 0.55), glass_material_id);

    Material mirror;
    mirror.ambient_color = Vector(0.01, 0.01, 0.01);
    mirror.diffuse_color = Vector(0.25, 0.25, 0.28);
    mirror.specular_color = Vector(0.95, 0.95, 0.95);
    mirror.albedo = Vector(0.05, 0.9, 0.0);
    mirror.specular_exponent = 192.0;
    const int mirror_material_id = scene.AddMaterial(mirror);
    scene.AddSphere(Sphere(Vector(1.8, 0.45, -4.8), 0.45), mirror_material_id);

    const int numRows = static_cast<int>(SampleRange(samples, &cursor, 2, 4));
    const int numCols = static_cast<int>(SampleRange(samples, &cursor, 2, 5));

    const double minX = -2.6;
    const double maxX = 2.6;
    const double minZ = -9.0;
    const double maxZ = -5.0;

    for (int row = 0; row < numRows; ++row) {
        for (int col = 0; col < numCols; ++col) {
            Material material = MakeMaterial(samples, &cursor);
            const int material_id = scene.AddMaterial(material);

            const double x = minX + (maxX - minX) * col / (numCols - 1) +
                             SampleRange(samples, &cursor, -0.2, 0.2);
            const double y = SampleRange(samples, &cursor, 0.9, 1.9);
            const double z = maxZ - (maxZ - minZ) * row / (numRows - 1) -
                             SampleRange(samples, &cursor, 0.0, 1.2);
            const double radius = SampleRange(samples, &cursor, 0.22, 0.5);

            scene.AddSphere(Sphere(Vector(x, y, z), radius), material_id);
        }
    }

    scene.AddLight(Light(Vector(-5.0, 7.0, -2.0), Vector(1.8, 1.7, 1.6)));
    scene.AddLight(Light(Vector(4.5, 6.5, -9.0), Vector(1.1, 1.2, 1.4)));

    return scene;
}