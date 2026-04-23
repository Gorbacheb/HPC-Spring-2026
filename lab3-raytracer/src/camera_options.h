#pragma once

#include "vector.h"

struct CameraOptions {
    int screen_width = 800;
    int screen_height = 600;
    double fov = 1.0471975511965976;  // 60 deg in radians

    Vector look_from = Vector(0.0, 0.0, 5.0);
    Vector look_to = Vector(0.0, 0.0, 0.0);
};
