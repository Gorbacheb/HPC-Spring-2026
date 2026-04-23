#include <catch2/catch_test_macros.hpp>
#include <catch2/matchers/catch_matchers_floating_point.hpp>

#include "camera.h"
#include "geometry.h"
#include "scene.h"

namespace {

auto WithinAbs(double target, double eps = 1e-9) {
    return Catch::Matchers::WithinAbs(target, eps);
}

}  // namespace

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

TEST_CASE("Camera center ray points to look direction", "[lab3][camera]") {
    CameraOptions options;
    options.screen_width = 800;
    options.screen_height = 600;
    options.look_from = Vector(0.0, 0.0, 5.0);
    options.look_to = Vector(0.0, 0.0, 0.0);

    Camera camera(options);
    const Ray center_ray = camera.GenerateRay(camera.Width() / 2, camera.Height() / 2);
    const Vector expected = options.look_to - options.look_from;

    const double cos_angle = DotProduct(center_ray.GetDirection(), expected) / Length(expected);
    REQUIRE(cos_angle > 0.999);
}

TEST_CASE("Sphere intersection exists in front of camera", "[lab3][geometry]") {
    const Ray ray(Vector(0.0, 0.0, 0.0), Vector(0.0, 0.0, -1.0));
    const Sphere sphere(Vector(0.0, 0.0, -5.0), 1.0);

    const auto hit = GetIntersection(ray, sphere);
    REQUIRE(hit.has_value());
    CHECK_THAT(hit->GetDistance(), WithinAbs(4.0));
    CHECK_THAT(hit->GetPosition()[0], WithinAbs(0.0));
    CHECK_THAT(hit->GetPosition()[1], WithinAbs(0.0));
    CHECK_THAT(hit->GetPosition()[2], WithinAbs(-4.0));
}

TEST_CASE("Sphere intersection misses when ray points away", "[lab3][geometry]") {
    const Ray ray(Vector(0.0, 0.0, 0.0), Vector(0.0, 0.0, 1.0));
    const Sphere sphere(Vector(0.0, 0.0, -5.0), 1.0);

    const auto hit = GetIntersection(ray, sphere);
    REQUIRE_FALSE(hit.has_value());
}

TEST_CASE("Sphere normal is unit and outward", "[lab3][geometry]") {
    const Sphere sphere(Vector(0.0, 0.0, -5.0), 1.0);
    const Ray ray(Vector(0.0, 0.0, 0.0), Vector(0.0, 0.0, -1.0));

    const auto hit = GetIntersection(ray, sphere);
    REQUIRE(hit.has_value());

    const Vector n = GetNormal(sphere, *hit);
    REQUIRE(Length(n) > 0.999);
    REQUIRE(Length(n) < 1.001);
    REQUIRE(DotProduct(n, hit->GetPosition() - sphere.GetCenter()) > 0.999);
}

TEST_CASE("Sphere tangent intersection is handled", "[lab3][geometry]") {
    const Ray ray(Vector(0.0, 1.0, 0.0), Vector(0.0, 0.0, -1.0));
    const Sphere sphere(Vector(0.0, 0.0, -5.0), 1.0);

    const auto hit = GetIntersection(ray, sphere);
    REQUIRE(hit.has_value());
    CHECK_THAT(hit->GetDistance(), WithinAbs(5.0));
    CHECK_THAT(hit->GetPosition()[0], WithinAbs(0.0));
    CHECK_THAT(hit->GetPosition()[1], WithinAbs(1.0));
    CHECK_THAT(hit->GetPosition()[2], WithinAbs(-5.0));
}

TEST_CASE("Sphere intersection from inside returns forward exit", "[lab3][geometry]") {
    const Ray ray(Vector(0.0, 0.0, -5.0), Vector(1.0, 0.0, 0.0));
    const Sphere sphere(Vector(0.0, 0.0, -5.0), 2.0);

    const auto hit = GetIntersection(ray, sphere);
    REQUIRE(hit.has_value());
    CHECK_THAT(hit->GetDistance(), WithinAbs(2.0));
    CHECK_THAT(hit->GetPosition()[0], WithinAbs(2.0));
    CHECK_THAT(hit->GetPosition()[1], WithinAbs(0.0));
    CHECK_THAT(hit->GetPosition()[2], WithinAbs(-5.0));
}

TEST_CASE("Triangle intersection works", "[lab3][geometry]") {
    const Triangle tri(
        Vector(-1.0, -1.0, -3.0),
        Vector(1.0, -1.0, -3.0),
        Vector(0.0, 1.0, -3.0)
    );
    const Ray ray(Vector(0.0, 0.0, 0.0), Vector(0.0, 0.0, -1.0));

    const auto hit = GetIntersection(ray, tri);
    REQUIRE(hit.has_value());
    CHECK_THAT(hit->GetDistance(), WithinAbs(3.0));
    CHECK_THAT(hit->GetPosition()[0], WithinAbs(0.0));
    CHECK_THAT(hit->GetPosition()[1], WithinAbs(0.0));
    CHECK_THAT(hit->GetPosition()[2], WithinAbs(-3.0));
}

TEST_CASE("Triangle intersection misses outside barycentric area", "[lab3][geometry]") {
    const Triangle tri(
        Vector(-1.0, -1.0, -3.0),
        Vector(1.0, -1.0, -3.0),
        Vector(0.0, 1.0, -3.0)
    );
    const Ray ray(Vector(2.0, 0.0, 0.0), Vector(0.0, 0.0, -1.0));

    const auto hit = GetIntersection(ray, tri);
    REQUIRE_FALSE(hit.has_value());
}

TEST_CASE("Triangle normal is unit and stable", "[lab3][geometry]") {
    const Triangle tri(
        Vector(-1.0, -1.0, -3.0),
        Vector(1.0, -1.0, -3.0),
        Vector(0.0, 1.0, -3.0)
    );
    const Ray ray(Vector(0.0, 0.0, 0.0), Vector(0.0, 0.0, -1.0));
    const auto hit = GetIntersection(ray, tri);
    REQUIRE(hit.has_value());

    const Vector n = GetNormal(tri, *hit);
    REQUIRE(Length(n) > 0.999);
    REQUIRE(Length(n) < 1.001);
    REQUIRE(std::fabs(n[2]) > 0.999);
}

TEST_CASE("Triangle intersection misses for ray parallel to plane", "[lab3][geometry]") {
    const Triangle tri(
        Vector(-1.0, -1.0, -3.0),
        Vector(1.0, -1.0, -3.0),
        Vector(0.0, 1.0, -3.0)
    );
    const Ray ray(Vector(0.0, 0.0, 0.0), Vector(1.0, 0.0, 0.0));

    const auto hit = GetIntersection(ray, tri);
    REQUIRE_FALSE(hit.has_value());
}

TEST_CASE("Scene stores objects, materials and lights coherently", "[lab3][scene]") {
    Scene scene;

    Material red;
    red.diffuse_color = Vector(0.9, 0.1, 0.1);
    const int red_id = scene.AddMaterial(red);

    Material green;
    green.diffuse_color = Vector(0.1, 0.9, 0.1);
    const int green_id = scene.AddMaterial(green);

    scene.AddSphere(Sphere(Vector(0.0, 0.0, -5.0), 1.0), red_id);
    scene.AddTriangle(
        Triangle(
            Vector(-1.0, -1.0, -3.0),
            Vector(1.0, -1.0, -3.0),
            Vector(0.0, 1.0, -3.0)
        ),
        green_id
    );
    scene.AddLight(Light(Vector(1.0, 2.0, 3.0), Vector(1.0, 1.0, 1.0)));

    REQUIRE(scene.GetSphereObjects().size() == 1);
    REQUIRE(scene.GetTriangleObjects().size() == 1);
    REQUIRE(scene.GetLights().size() == 1);

    REQUIRE(scene.GetSphereObjects()[0].material != nullptr);
    REQUIRE(scene.GetTriangleObjects()[0].material != nullptr);
}
