#pragma once

#include "vector.h"

class Intersection {
public:
    __host__ __device__ Intersection() : distance_(0.0), position_(0.0, 0.0, 0.0) {
    }

    __host__ __device__ Intersection(double distance, const Vector& position)
        : distance_(distance), position_(position) {
    }

    [[nodiscard]] __host__ __device__ double GetDistance() const {
        return distance_;
    }

    [[nodiscard]] __host__ __device__ const Vector& GetPosition() const {
        return position_;
    }

private:
    double distance_;
    Vector position_;
};
