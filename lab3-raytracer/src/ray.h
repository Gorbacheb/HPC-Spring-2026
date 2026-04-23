#pragma once

#include "vector.h"

class Ray {
public:
    Ray() = default;

    Ray(const Vector& origin, const Vector& direction) : origin_(origin), direction_(direction) {
        direction_.Normalize();
    }

    [[nodiscard]] const Vector& GetOrigin() const {
        return origin_;
    }

    [[nodiscard]] const Vector& GetDirection() const {
        return direction_;
    }

    [[nodiscard]] Vector GetPosition(double t) const {
        return origin_ + direction_ * t;
    }

private:
    Vector origin_;
    Vector direction_;
};
