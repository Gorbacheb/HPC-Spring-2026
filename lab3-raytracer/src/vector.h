#pragma once

#include <cuda_runtime.h>

#include <cmath>

class Vector;
__host__ __device__ double Length(const Vector& v);

class Vector {
public:
    __host__ __device__ Vector() : x_(0.0), y_(0.0), z_(0.0) {
    }

    __host__ __device__ Vector(double x, double y, double z) : x_(x), y_(y), z_(z) {
    }

    __host__ __device__ double operator[](int i) const {
        if (i == 0) {
            return x_;
        }
        if (i == 1) {
            return y_;
        }
        return z_;
    }

    __host__ __device__ double& operator[](int i) {
        if (i == 0) {
            return x_;
        }
        if (i == 1) {
            return y_;
        }
        return z_;
    }

    __host__ __device__ Vector operator+(const Vector& rhs) const {
        return {x_ + rhs.x_, y_ + rhs.y_, z_ + rhs.z_};
    }

    __host__ __device__ Vector operator-(const Vector& rhs) const {
        return {x_ - rhs.x_, y_ - rhs.y_, z_ - rhs.z_};
    }

    __host__ __device__ Vector operator-() const {
        return {-x_, -y_, -z_};
    }

    __host__ __device__ Vector operator*(double k) const {
        return {x_ * k, y_ * k, z_ * k};
    }

    __host__ __device__ Vector operator/(double k) const {
        return {x_ / k, y_ / k, z_ / k};
    }

    __host__ __device__ Vector& operator+=(const Vector& rhs) {
        x_ += rhs.x_;
        y_ += rhs.y_;
        z_ += rhs.z_;
        return *this;
    }

    __host__ __device__ void Normalize() {
        const double len = Length(*this);
        if (len < 1e-12) {
            return;
        }
        x_ /= len;
        y_ /= len;
        z_ /= len;
    }

private:
    double x_;
    double y_;
    double z_;
};

__host__ __device__ inline Vector operator*(double k, const Vector& v) {
    return v * k;
}

__host__ __device__ inline double DotProduct(const Vector& a, const Vector& b) {
    return a[0] * b[0] + a[1] * b[1] + a[2] * b[2];
}

__host__ __device__ inline Vector CrossProduct(const Vector& a, const Vector& b) {
    return {a[1] * b[2] - a[2] * b[1], a[2] * b[0] - a[0] * b[2], a[0] * b[1] - a[1] * b[0]};
}

__host__ __device__ inline double Length(const Vector& v) {
    return sqrt(DotProduct(v, v));
}

__host__ __device__ inline Vector Reflect(const Vector& ray, const Vector& normal) {
    return ray - 2.0 * DotProduct(ray, normal) * normal;
}

__host__ __device__ inline bool TryRefract(const Vector& unit_direction, const Vector& normal,
                                           double refraction_ratio, Vector* refracted) {
    double cos_theta = DotProduct(-unit_direction, normal);
    if (cos_theta > 1.0) {
        cos_theta = 1.0;
    }

    const Vector r_out_perp = refraction_ratio * (unit_direction + cos_theta * normal);
    const double parallel_sq = 1.0 - DotProduct(r_out_perp, r_out_perp);
    if (parallel_sq < 0.0) {
        return false;
    }

    *refracted = r_out_perp - sqrt(parallel_sq) * normal;
    return true;
}

__host__ __device__ inline Vector HadamardProduct(const Vector& lhs, const Vector& rhs) {
    return {lhs[0] * rhs[0], lhs[1] * rhs[1], lhs[2] * rhs[2]};
}