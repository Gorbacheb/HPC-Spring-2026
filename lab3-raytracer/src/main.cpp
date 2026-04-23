#include <iostream>

#include "camera.h"

int main() {
    CameraOptions options;
    options.screen_width = 800;
    options.screen_height = 600;
    options.look_from = Vector(0.0, 1.0, 5.0);
    options.look_to = Vector(0.0, 0.0, 0.0);

    Camera camera(options);
    const Ray center_ray = camera.GenerateRay(camera.Width() / 2, camera.Height() / 2);
    const Vector d = center_ray.GetDirection();

    std::cout << "center ray direction = (" << d[0] << ", " << d[1] << ", " << d[2] << ")\n";
    return 0;
}
