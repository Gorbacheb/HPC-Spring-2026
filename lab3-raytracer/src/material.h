#pragma once

#include <array>

#include "vector.h"

struct Material {
    Vector ambient_color = Vector(0.05, 0.05, 0.05);
    Vector diffuse_color = Vector(0.70, 0.70, 0.70);
    Vector specular_color = Vector(0.30, 0.30, 0.30);
    Vector intensity = Vector(0.0, 0.0, 0.0);

    std::array<double, 3> albedo = {1.0, 0.0, 0.0};
    double specular_exponent = 32.0;
    double refraction_index = 1.0;
};
