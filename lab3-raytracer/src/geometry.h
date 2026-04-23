#pragma once

#include <optional>

#include "intersection.h"
#include "ray.h"
#include "vector.h"

class Sphere {
public:
    Sphere() : center_(0.0, 0.0, 0.0), radius_(1.0) {
    }

    Sphere(const Vector& center, double radius) : center_(center), radius_(radius) {
    }

    [[nodiscard]] const Vector& GetCenter() const {
        return center_;
    }

    [[nodiscard]] double GetRadius() const {
        return radius_;
    }

private:
    Vector center_;
    double radius_;
};

class Triangle {
public:
    Triangle() : a_(0.0, 0.0, 0.0), b_(1.0, 0.0, 0.0), c_(0.0, 1.0, 0.0) {
    }

    Triangle(const Vector& a, const Vector& b, const Vector& c) : a_(a), b_(b), c_(c) {
    }

    [[nodiscard]] const Vector& A() const {
        return a_;
    }

    [[nodiscard]] const Vector& B() const {
        return b_;
    }

    [[nodiscard]] const Vector& C() const {
        return c_;
    }

private:
    Vector a_;
    Vector b_;
    Vector c_;
};

inline std::optional<Intersection> GetIntersection(const Ray& ray, const Sphere& sphere) {
    const Vector oc = ray.GetOrigin() - sphere.GetCenter();
    const double a = DotProduct(ray.GetDirection(), ray.GetDirection());
    const double b = 2.0 * DotProduct(oc, ray.GetDirection());
    const double c = DotProduct(oc, oc) - sphere.GetRadius() * sphere.GetRadius();
    const double d = b * b - 4.0 * a * c;

    if (d < 0.0) {
        return std::nullopt;
    }

    const double sd = std::sqrt(d);
    double t = (-b - sd) / (2.0 * a);
    if (t <= 1e-6) {
        t = (-b + sd) / (2.0 * a);
        if (t <= 1e-6) {
            return std::nullopt;
        }
    }

    return Intersection(t, ray.GetPosition(t));
}

inline std::optional<Intersection> GetIntersection(const Ray& ray, const Triangle& tri) {
    const double eps = 1e-8;
    const Vector edge1 = tri.B() - tri.A();
    const Vector edge2 = tri.C() - tri.A();

    const Vector h = CrossProduct(ray.GetDirection(), edge2);
    const double det = DotProduct(edge1, h);
    if (det > -eps && det < eps) {
        return std::nullopt;
    }

    const double inv_det = 1.0 / det;
    const Vector s = ray.GetOrigin() - tri.A();
    const double u = inv_det * DotProduct(s, h);
    if (u < 0.0 || u > 1.0) {
        return std::nullopt;
    }

    const Vector q = CrossProduct(s, edge1);
    const double v = inv_det * DotProduct(ray.GetDirection(), q);
    if (v < 0.0 || u + v > 1.0) {
        return std::nullopt;
    }

    const double t = inv_det * DotProduct(edge2, q);
    if (t <= eps) {
        return std::nullopt;
    }

    return Intersection(t, ray.GetPosition(t));
}

inline Vector GetNormal(const Sphere& sphere, const Intersection& intersection) {
    Vector n = intersection.GetPosition() - sphere.GetCenter();
    n.Normalize();
    return n;
}

inline Vector GetNormal(const Triangle& tri, const Intersection&) {
    Vector n = CrossProduct(tri.B() - tri.A(), tri.C() - tri.A());
    n.Normalize();
    return n;
}
