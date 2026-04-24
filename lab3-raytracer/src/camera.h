#pragma once

#include <cmath>

#include "camera_options.h"
#include "ray.h"
#include "vector.h"

class Camera {
public:
    explicit Camera(const CameraOptions& options)
        : width_(options.screen_width), height_(options.screen_height), origin_(options.look_from) {
        // https://www.scratchapixel.com/lessons/mathematics-physics-for-computer-graphics/lookat-function/framing-lookat-function.html
        forward_ = options.look_to - options.look_from;
        forward_.Normalize();

        // защита от случая, когда forward_ почти || (0,1,0)
        const Vector world_up(0.0, 1.0, 0.0);
        const double eps = 1e-9;
        if (std::fabs(DotProduct(forward_, world_up)) > 1.0 - eps) {
            // камера смотрит почти вверх или вниз - выберем ось X как резервную
            right_ = Vector(1.0, 0.0, 0.0);
        } else {
            right_ = CrossProduct(forward_, world_up);
        }
        right_.Normalize();

        up_ = CrossProduct(right_, forward_);
        up_.Normalize();

        aspect_ = static_cast<double>(width_) / height_;
        scale_ = std::tan(options.fov * 0.5);
    }

    [[nodiscard]] __host__ __device__ Ray GenerateRay(int x, int y) const {
        // https://www.scratchapixel.com/lessons/3d-basic-rendering/ray-tracing-generating-camera-rays/generating-camera-rays.html

        double px = (2.0 * (x + 0.5) / width_ - 1.0) * aspect_ * scale_;
        double py = (1.0 - 2.0 * (y + 0.5) / height_) * scale_;

        Vector dir = forward_ + px * right_ + py * up_;
        dir.Normalize();

        return {origin_, dir};
    }

    [[nodiscard]] __host__ __device__ int Width() const {
        return width_;
    }

    [[nodiscard]] __host__ __device__ int Height() const {
        return height_;
    }

private:
    int width_;
    int height_;
    Vector origin_;
    Vector forward_;
    Vector right_;
    Vector up_;
    double aspect_;
    double scale_;
};
