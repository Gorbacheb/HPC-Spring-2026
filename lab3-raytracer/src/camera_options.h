#pragma once

#include "vector.h"

struct CameraOptions {
    int screen_width = 800;
    int screen_height = 600;
    double fov = std::acos(-1) / 2;  // pi/2

    Vector look_from = Vector(0.0, 0.0, 0.0);
    Vector look_to = Vector(0.0, 0.0, -1.0);
};
