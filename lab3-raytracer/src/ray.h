#pragma once

#include "vector.h"

class Ray {
public:
    __host__ __device__ Ray() = default;

    __host__ __device__ Ray(const Vector& origin, const Vector& direction) : origin_(origin), direction_(direction) {
        direction_.Normalize();
    }

    [[nodiscard]] __host__ __device__ const Vector& GetOrigin() const {
        return origin_;
    }

    [[nodiscard]] __host__ __device__ const Vector& GetDirection() const {
        return direction_;
    }

    [[nodiscard]] __host__ __device__ Vector GetPosition(double t) const {
        return origin_ + direction_ * t;
    }

private:
    Vector origin_;
    Vector direction_;
};
