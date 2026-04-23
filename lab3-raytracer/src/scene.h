#pragma once

#include <vector>

#include "geometry.h"
#include "light.h"
#include "material.h"

struct SphereObject {
    Sphere sphere;
    const Material* material = nullptr;

    [[nodiscard]] Vector GetNormal(const Intersection& intersection) const {
        return ::GetNormal(sphere, intersection);
    }
};

struct TriangleObject {
    Triangle triangle;
    const Material* material = nullptr;

    [[nodiscard]] Vector GetNormal(const Intersection& intersection) const {
        return ::GetNormal(triangle, intersection);
    }
};

class Scene {
public:
    int AddMaterial(const Material& material) {
        materials_.push_back(material);
        return static_cast<int>(materials_.size()) - 1;
    }

    void AddSphere(const Sphere& sphere, int material_id) {
        SphereObject obj;
        obj.sphere = sphere;
        obj.material = &materials_[material_id];
        sphere_objects_.push_back(obj);
    }

    void AddTriangle(const Triangle& triangle, int material_id) {
        TriangleObject obj;
        obj.triangle = triangle;
        obj.material = &materials_[material_id];
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

private:
    std::vector<Material> materials_;
    std::vector<SphereObject> sphere_objects_;
    std::vector<TriangleObject> triangle_objects_;
    std::vector<Light> lights_;
};
