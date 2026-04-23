#pragma once

#include "vector.h"

class Intersection {
public:
    Intersection() : distance_(0.0), position_(0.0, 0.0, 0.0) {
    }

    Intersection(double distance, const Vector& position)
        : distance_(distance), position_(position) {
    }

    [[nodiscard]] double GetDistance() const {
        return distance_;
    }

    [[nodiscard]] const Vector& GetPosition() const {
        return position_;
    }

private:
    double distance_;
    Vector position_;
};
