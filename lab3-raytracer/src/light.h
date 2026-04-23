#pragma once

#include "vector.h"

struct Light {
    Vector position;
    Vector intensity;

    Light() : position(0.0, 0.0, 0.0), intensity(1.0, 1.0, 1.0) {
    }

    Light(const Vector& p, const Vector& i) : position(p), intensity(i) {
    }
};
