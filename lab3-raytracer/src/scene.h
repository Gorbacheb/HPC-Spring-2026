#pragma once

#include <deque>
#include <optional>
#include <string>
#include <unordered_map>
#include <utility>
#include <vector>

#include <cuda_runtime.h>

#include "cuda_utils.h"
#include "geometry.h"
#include "light.h"
#include "material.h"

struct SphereObject {
    Sphere sphere;
    int material_id = -1;
};

struct TriangleObject {
    Triangle triangle;
    int material_id = -1;
};

class Scene {
public:
    Scene() = default;

    Scene(const Scene& other)
        : materials_(other.materials_),
          material_name_to_id_(other.material_name_to_id_),
          sphere_objects_(other.sphere_objects_),
          triangle_objects_(other.triangle_objects_),
          lights_(other.lights_) {}

    Scene(Scene&& other) noexcept
        : materials_(std::move(other.materials_)),
          material_name_to_id_(std::move(other.material_name_to_id_)),
          sphere_objects_(std::move(other.sphere_objects_)),
          triangle_objects_(std::move(other.triangle_objects_)),
          lights_(std::move(other.lights_)) {}

    Scene& operator=(const Scene& other) {
        if (this == &other) {
            return *this;
        }
        materials_ = other.materials_;
        material_name_to_id_ = other.material_name_to_id_;
        sphere_objects_ = other.sphere_objects_;
        triangle_objects_ = other.triangle_objects_;
        lights_ = other.lights_;
        return *this;
    }

    Scene& operator=(Scene&& other) noexcept {
        if (this == &other) {
            return *this;
        }
        materials_ = std::move(other.materials_);
        material_name_to_id_ = std::move(other.material_name_to_id_);
        sphere_objects_ = std::move(other.sphere_objects_);
        triangle_objects_ = std::move(other.triangle_objects_);
        lights_ = std::move(other.lights_);
        return *this;
    }

    struct DeviceSceneStorage {
        Material* materials = nullptr;
        int material_count = 0;
        Sphere* spheres = nullptr;
        int* sphere_material_ids = nullptr;
        int sphere_count = 0;
        Triangle* triangles = nullptr;
        int* triangle_material_ids = nullptr;
        int triangle_count = 0;
        Light* lights = nullptr;
        int light_count = 0;

        void Release() {
            if (lights != nullptr) {
                CUDA_CHECK(cudaFree(lights));
                lights = nullptr;
            }
            if (triangle_material_ids != nullptr) {
                CUDA_CHECK(cudaFree(triangle_material_ids));
                triangle_material_ids = nullptr;
            }
            if (triangles != nullptr) {
                CUDA_CHECK(cudaFree(triangles));
                triangles = nullptr;
            }
            if (sphere_material_ids != nullptr) {
                CUDA_CHECK(cudaFree(sphere_material_ids));
                sphere_material_ids = nullptr;
            }
            if (spheres != nullptr) {
                CUDA_CHECK(cudaFree(spheres));
                spheres = nullptr;
            }
            if (materials != nullptr) {
                CUDA_CHECK(cudaFree(materials));
                materials = nullptr;
            }
            material_count = 0;
            sphere_count = 0;
            triangle_count = 0;
            light_count = 0;
        }
    };

    int AddMaterial(const Material& material) {
        materials_.push_back(material);
        return static_cast<int>(materials_.size()) - 1;
    }

    int AddMaterial(const std::string& name, const Material& material) {
        const int id = AddMaterial(material);
        material_name_to_id_[name] = id;
        return id;
    }

    void AddSphere(const Sphere& sphere, int material_id) {
        SphereObject obj;
        obj.sphere = sphere;
        obj.material_id = material_id;
        sphere_objects_.push_back(obj);
    }

    void AddTriangle(const Triangle& triangle, int material_id) {
        TriangleObject obj;
        obj.triangle = triangle;
        obj.material_id = material_id;
        triangle_objects_.push_back(obj);
    }

    void AddLight(const Light& light) {
        lights_.push_back(light);
    }

    [[nodiscard]] const std::vector<SphereObject>& GetSphereObjects() const {
        return sphere_objects_;
    }

    [[nodiscard]] const std::vector<TriangleObject>& GetTriangleObjects() const {
        return triangle_objects_;
    }

    [[nodiscard]] const std::vector<Light>& GetLights() const {
        return lights_;
    }

    [[nodiscard]] const std::deque<Material>& GetMaterials() const {
        return materials_;
    }

    [[nodiscard]] std::optional<int> FindMaterialId(const std::string& name) const {
        const auto it = material_name_to_id_.find(name);
        if (it == material_name_to_id_.end()) {
            return std::nullopt;
        }
        return it->second;
    }

    [[nodiscard]] DeviceSceneStorage UploadScene() const {
        DeviceSceneStorage storage;

        std::vector<Material> materials(materials_.begin(), materials_.end());
        std::vector<Sphere> spheres;
        std::vector<int> sphere_material_ids;
        spheres.reserve(sphere_objects_.size());
        sphere_material_ids.reserve(sphere_objects_.size());
        for (const auto& sphere_object : sphere_objects_) {
            spheres.push_back(sphere_object.sphere);
            sphere_material_ids.push_back(sphere_object.material_id);
        }

        std::vector<Triangle> triangles;
        std::vector<int> triangle_material_ids;
        triangles.reserve(triangle_objects_.size());
        triangle_material_ids.reserve(triangle_objects_.size());
        for (const auto& triangle_object : triangle_objects_) {
            triangles.push_back(triangle_object.triangle);
            triangle_material_ids.push_back(triangle_object.material_id);
        }

        std::vector<Light> lights(lights_.begin(), lights_.end());

        storage.materials = UploadVector(materials);
        storage.material_count = static_cast<int>(materials.size());
        storage.spheres = UploadVector(spheres);
        storage.sphere_material_ids = UploadVector(sphere_material_ids);
        storage.sphere_count = static_cast<int>(spheres.size());
        storage.triangles = UploadVector(triangles);
        storage.triangle_material_ids = UploadVector(triangle_material_ids);
        storage.triangle_count = static_cast<int>(triangles.size());
        storage.lights = UploadVector(lights);
        storage.light_count = static_cast<int>(lights.size());
        return storage;
    }

private:
    template <typename T>
    static T* UploadVector(const std::vector<T>& host) {
        if (host.empty()) {
            return nullptr;
        }

        T* device = nullptr;
        CUDA_CHECK(cudaMalloc(&device, host.size() * sizeof(T)));
        CUDA_CHECK(cudaMemcpy(device, host.data(), host.size() * sizeof(T), cudaMemcpyHostToDevice));
        return device;
    }

    std::deque<Material> materials_;
    std::unordered_map<std::string, int> material_name_to_id_;
    std::vector<SphereObject> sphere_objects_;
    std::vector<TriangleObject> triangle_objects_;
    std::vector<Light> lights_;
};
