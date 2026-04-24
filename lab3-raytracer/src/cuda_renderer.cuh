#pragma once

#include <algorithm>
#include <cmath>
#include <iostream>
#include <string>
#include <vector>

#include <cuda_runtime.h>

#include "bmp_utils.h"

#include "camera.h"
#include "cuda_utils.h"
#include "geometry.h"
#include "light.h"
#include "material.h"
#include "ray.h"
#include "scene.h"
#include "vector.h"

constexpr double kEps = 1e-6;

struct NearestIntersectionResult {
    Intersection intersection;
    Vector normal;
    const Material* material = nullptr;
    bool is_sphere = false;
    Sphere sphere;

    __host__ __device__ bool HasValue() const {
        return material != nullptr;
    }

    __host__ __device__ const Intersection& GetIntersection() const {
        return intersection;
    }

    __host__ __device__ const Vector& GetNormal() const {
        return normal;
    }

    __host__ __device__ const Material& GetMaterial() const {
        return *material;
    }
};

// todo было бы прикольно сделать BVH, что даст логарифм вместо линии
__host__ __device__ inline NearestIntersectionResult NearestIntersection(
    const Ray& ray, const Scene::DeviceSceneStorage& scene) {
    NearestIntersectionResult result;
    double nearest_distance = 0.0;

    for (int i = 0; i < scene.sphere_count; ++i) {
        Intersection candidate;
        if (TryGetIntersection(ray, scene.spheres[i], &candidate) &&
            (!result.HasValue() || candidate.GetDistance() < nearest_distance)) {
            result.intersection = candidate;
            const Vector outward_normal = GetNormal(scene.spheres[i], candidate);
            result.normal = DotProduct(ray.GetDirection(), outward_normal) < 0.0 ? outward_normal
                                                                                 : -outward_normal;
            result.material = &scene.materials[scene.sphere_material_ids[i]];
            result.is_sphere = true;
            result.sphere = scene.spheres[i];
            nearest_distance = candidate.GetDistance();
        }
    }

    for (int i = 0; i < scene.triangle_count; ++i) {
        Intersection candidate;
        if (TryGetIntersection(ray, scene.triangles[i], &candidate) &&
            (!result.HasValue() || candidate.GetDistance() < nearest_distance)) {
            result.intersection = candidate;
            const Vector outward_normal = GetNormal(scene.triangles[i], candidate);
            result.normal = DotProduct(ray.GetDirection(), outward_normal) < 0.0 ? outward_normal
                                                                                 : -outward_normal;
            result.normal.Normalize();
            result.material = &scene.materials[scene.triangle_material_ids[i]];
            result.is_sphere = false;
            nearest_distance = candidate.GetDistance();
        }
    }

    return result;
}

__host__ __device__ inline bool IsShadowed(const Ray& ray, const Scene::DeviceSceneStorage& scene,
                                           double max_distance) {
    const NearestIntersectionResult nearest = NearestIntersection(ray, scene);
    if (!nearest.HasValue()) {
        return false;
    }
    return nearest.GetIntersection().GetDistance() < max_distance - kEps;
}

__host__ __device__ inline Vector TraceRay(const Ray& ray_in,
                                           const Scene::DeviceSceneStorage& scene, int max_depth) {
    struct TraceFrame {
        Ray ray;
        int depth;
        double weight;
    };

    // сделал свой стек, т.к. рекурсия не заработала
    constexpr int kMaxTraceStack = 256;
    TraceFrame stack[kMaxTraceStack];
    int stack_size = 0;

    stack[stack_size++] = {ray_in, max_depth, 1.0};

    Vector color(0.0, 0.0, 0.0);

    while (stack_size > 0) {
        const TraceFrame frame = stack[--stack_size];
        const NearestIntersectionResult nearest = NearestIntersection(frame.ray, scene);
        if (!nearest.HasValue()) {
            continue;
        }

        const Intersection& nearest_intersection = nearest.GetIntersection();
        const Vector& nearest_normal = nearest.GetNormal();
        const Material& material = nearest.GetMaterial();

        Vector local_color = material.ambient_color + material.intensity;

        const Vector reflected_direction = Reflect(frame.ray.GetDirection(), nearest_normal);

        // смещаю начало отраженного луча, чтобы избежать самопересечения (тут и далее, где kEps)
        const Ray reflected_ray(nearest_intersection.GetPosition() + 2.0 * nearest_normal * kEps,
                                reflected_direction);

        for (int i = 0; i < scene.light_count; ++i) {
            const Light& light = scene.lights[i];
            const Vector to_light = light.position - nearest_intersection.GetPosition();
            const double light_distance = Length(to_light);
            const Vector light_direction = to_light / light_distance;
            const Ray light_ray(nearest_intersection.GetPosition() + nearest_normal * kEps,
                                light_direction);

            if (IsShadowed(light_ray, scene, light_distance)) {
                continue;
            }

            // диффузная составляющая
            // яркость пропорциональна косинусу угла между нормалью и направлением на свет
            // diffuse = light_color * diffuse_color * max(0, N·L)
            double diffuse_factor = DotProduct(light_direction, nearest_normal);
            if (diffuse_factor < 0.0) {
                diffuse_factor = 0.0;
            }

            const Vector diffuse = HadamardProduct(light.intensity, material.diffuse_color);
            local_color += material.albedo[0] * diffuse_factor * diffuse;

            // зеркальная составляющая:
            // specular = light_color * specular_color * (R·V)^shininess
            double specular_factor = DotProduct(light_direction, reflected_ray.GetDirection());
            if (specular_factor < 0.0) {
                specular_factor = 0.0;
            }

            const Vector specular = HadamardProduct(light.intensity, material.specular_color);
            local_color +=
                material.albedo[0] * pow(specular_factor, material.specular_exponent) * specular;
        }

        color += frame.weight * local_color;

        if (frame.depth == 0) {
            continue;
        }

        // 1) зеркальное отражение
        if (material.albedo[1] > 0.0 && stack_size < kMaxTraceStack) {
            bool reflect_allowed = true;
            // для сфер: не допускаю отражение, если отраженный луч остается внутри сферы
            if (nearest.is_sphere) {
                reflect_allowed = Length(reflected_ray.GetOrigin() - nearest.sphere.GetCenter()) >
                                  nearest.sphere.GetRadius() + kEps;
            }
            if (reflect_allowed) {
                stack[stack_size++] = {reflected_ray, frame.depth - 1,
                                       frame.weight * material.albedo[1]};
            }
        }

        // 2) преломление
        if (material.albedo[2] > 0.0 && material.refraction_index > 1.0) {
            double refraction_ratio = 1.0 / material.refraction_index;
            bool inside_sphere = false;
            if (nearest.is_sphere) {
                inside_sphere = Length(reflected_ray.GetOrigin() - nearest.sphere.GetCenter()) <
                                nearest.sphere.GetRadius() - kEps;
                if (inside_sphere) {
                    refraction_ratio = material.refraction_index;
                }
            }

            Vector refracted_direction;
            // вычисление преломленного направления
            if (TryRefract(frame.ray.GetDirection(), nearest_normal, refraction_ratio,
                           &refracted_direction)) {
                const Ray refracted_ray(
                    nearest_intersection.GetPosition() - 2.0 * nearest_normal * kEps,
                    refracted_direction);
                const double refracted_weight =
                    inside_sphere ? frame.weight : (frame.weight * material.albedo[2]);
                if (stack_size < kMaxTraceStack) {
                    stack[stack_size++] = {refracted_ray, frame.depth - 1, refracted_weight};
                }
            }
        }
    }

    return color;
}

__global__ void RenderKernel(Camera camera, Scene::DeviceSceneStorage scene, int depth,
                             Vector* out_pixels) {
    const int x = blockIdx.x * blockDim.x + threadIdx.x;
    const int y = blockIdx.y * blockDim.y + threadIdx.y;
    if (x >= camera.Width() || y >= camera.Height()) {
        return;
    }

    const Ray ray = camera.GenerateRay(x, y);
    out_pixels[static_cast<size_t>(y) * static_cast<size_t>(camera.Width()) +
               static_cast<size_t>(x)] = TraceRay(ray, scene, depth);
}

inline double RenderSceneToBmp(const Camera& camera, const Scene::DeviceSceneStorage& scene,
                               int depth, const std::string& output_path) {
    Vector* d_pixels = nullptr;
    const size_t pixel_count =
        static_cast<size_t>(camera.Width()) * static_cast<size_t>(camera.Height());
    CUDA_CHECK(cudaMalloc(&d_pixels, pixel_count * sizeof(Vector)));

    const dim3 block(16, 16);
    const dim3 grid((camera.Width() + block.x - 1) / block.x,
                    (camera.Height() + block.y - 1) / block.y);

    cudaEvent_t start = nullptr;
    cudaEvent_t stop = nullptr;
    CUDA_CHECK(cudaEventCreate(&start));
    CUDA_CHECK(cudaEventCreate(&stop));

    CUDA_CHECK(cudaEventRecord(start));
    RenderKernel<<<grid, block>>>(camera, scene, depth, d_pixels);
    CUDA_CHECK(cudaGetLastError());
    CUDA_CHECK(cudaEventRecord(stop));
    CUDA_CHECK(cudaEventSynchronize(stop));

    float kernel_ms = 0.0f;
    CUDA_CHECK(cudaEventElapsedTime(&kernel_ms, start, stop));

    std::vector<Vector> pixels(pixel_count);
    CUDA_CHECK(
        cudaMemcpy(pixels.data(), d_pixels, pixel_count * sizeof(Vector), cudaMemcpyDeviceToHost));
    SaveBmp(output_path, pixels, camera.Width(), camera.Height());

    std::cout << "GPU render time: " << kernel_ms << " ms\n";
    std::cout << "Image saved to " << output_path << '\n';

    CUDA_CHECK(cudaEventDestroy(stop));
    CUDA_CHECK(cudaEventDestroy(start));
    CUDA_CHECK(cudaFree(d_pixels));
    return kernel_ms;
}
