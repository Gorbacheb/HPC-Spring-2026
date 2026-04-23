#include <catch2/catch_test_macros.hpp>

#include "camera.h"

TEST_CASE("Camera center ray is normalized", "[lab3][camera]") {
    CameraOptions options;
    options.screen_width = 640;
    options.screen_height = 480;

    Camera camera(options);
    const Ray ray = camera.GenerateRay(camera.Width() / 2, camera.Height() / 2);
    const Vector d = ray.GetDirection();
    const double len = Length(d);

    REQUIRE(len > 0.999);
    REQUIRE(len < 1.001);
}
