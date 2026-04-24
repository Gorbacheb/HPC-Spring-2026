#pragma once

#include <optional>

#include "intersection.h"
#include "ray.h"
#include "vector.h"

class Sphere {
public:
    __host__ __device__ Sphere() : center_(0.0, 0.0, 0.0), radius_(1.0) {
    }

    __host__ __device__ Sphere(const Vector& center, double radius)
        : center_(center), radius_(radius) {
    }

    [[nodiscard]] __host__ __device__ const Vector& GetCenter() const {
        return center_;
    }

    [[nodiscard]] __host__ __device__ double GetRadius() const {
        return radius_;
    }

private:
    Vector center_;
    double radius_;
};

class Triangle {
public:
    __host__ __device__ Triangle()
        : a_(0.0, 0.0, 0.0),
          b_(1.0, 0.0, 0.0),
          c_(0.0, 1.0, 0.0),
          na_(0.0, 0.0, 0.0),
          nb_(0.0, 0.0, 0.0),
          nc_(0.0, 0.0, 0.0),
          has_vertex_normals_(false) {
    }

    __host__ __device__ Triangle(const Vector& a, const Vector& b, const Vector& c)
        : a_(a),
          b_(b),
          c_(c),
          na_(0.0, 0.0, 0.0),
          nb_(0.0, 0.0, 0.0),
          nc_(0.0, 0.0, 0.0),
          has_vertex_normals_(false) {
    }

    __host__ __device__ Triangle(const Vector& a, const Vector& b, const Vector& c,
                                 const Vector& na, const Vector& nb, const Vector& nc)
        : a_(a), b_(b), c_(c), na_(na), nb_(nb), nc_(nc), has_vertex_normals_(true) {
    }

    [[nodiscard]] __host__ __device__ const Vector& A() const {
        return a_;
    }

    [[nodiscard]] __host__ __device__ const Vector& B() const {
        return b_;
    }

    [[nodiscard]] __host__ __device__ const Vector& C() const {
        return c_;
    }

    [[nodiscard]] __host__ __device__ const Vector& NormalA() const {
        return na_;
    }

    [[nodiscard]] __host__ __device__ const Vector& NormalB() const {
        return nb_;
    }

    [[nodiscard]] __host__ __device__ const Vector& NormalC() const {
        return nc_;
    }

    [[nodiscard]] __host__ __device__ bool HasVertexNormals() const {
        return has_vertex_normals_;
    }

private:
    Vector a_;
    Vector b_;
    Vector c_;
    Vector na_;
    Vector nb_;
    Vector nc_;
    bool has_vertex_normals_;
};

__host__ __device__ inline bool TryGetIntersection(const Ray& ray, const Sphere& sphere,
                                                   Intersection* intersection) {
    const Vector oc = ray.GetOrigin() - sphere.GetCenter();
    const double a = DotProduct(ray.GetDirection(), ray.GetDirection());
    const double b = 2.0 * DotProduct(oc, ray.GetDirection());
    const double c = DotProduct(oc, oc) - sphere.GetRadius() * sphere.GetRadius();
    const double d = b * b - 4.0 * a * c;

    if (d < 0.0) {
        return false;
    }

    const double sd = sqrt(d);
    double t = (-b - sd) / (2.0 * a);
    if (t <= 1e-6) {
        t = (-b + sd) / (2.0 * a);
        if (t <= 1e-6) {
            return false;
        }
    }

    *intersection = Intersection(t, ray.GetPosition(t));
    return true;
}

inline std::optional<Intersection> GetIntersection(const Ray& ray, const Sphere& sphere) {
    Intersection intersection;
    if (!TryGetIntersection(ray, sphere, &intersection)) {
        return std::nullopt;
    }
    return intersection;
}

__host__ __device__ inline bool TryGetIntersection(const Ray& ray, const Triangle& tri,
                                                   Intersection* intersection) {
    const double eps = 1e-8;
    const Vector edge1 = tri.B() - tri.A();
    const Vector edge2 = tri.C() - tri.A();

    const Vector h = CrossProduct(ray.GetDirection(), edge2);
    const double det = DotProduct(edge1, h);
    if (det > -eps && det < eps) {
        return false;
    }

    const double inv_det = 1.0 / det;
    const Vector s = ray.GetOrigin() - tri.A();
    const double u = inv_det * DotProduct(s, h);
    if (u < 0.0 || u > 1.0) {
        return false;
    }

    const Vector q = CrossProduct(s, edge1);
    const double v = inv_det * DotProduct(ray.GetDirection(), q);
    if (v < 0.0 || u + v > 1.0) {
        return false;
    }

    const double t = inv_det * DotProduct(edge2, q);
    if (t <= eps) {
        return false;
    }

    *intersection = Intersection(t, ray.GetPosition(t));
    return true;
}

inline std::optional<Intersection> GetIntersection(const Ray& ray, const Triangle& tri) {
    Intersection intersection;
    if (!TryGetIntersection(ray, tri, &intersection)) {
        return std::nullopt;
    }
    return intersection;
}

__host__ __device__ inline Vector GetNormal(const Sphere& sphere, const Intersection& intersection) {
    Vector n = intersection.GetPosition() - sphere.GetCenter();
    n.Normalize();
    return n;
}

__host__ __device__ inline Vector GetBarycentricCoords(const Triangle& tri, const Vector& point) {
    const Vector v0 = tri.B() - tri.A();
    const Vector v1 = tri.C() - tri.A();
    const Vector v2 = point - tri.A();

    const double d00 = DotProduct(v0, v0);
    const double d01 = DotProduct(v0, v1);
    const double d11 = DotProduct(v1, v1);
    const double d20 = DotProduct(v2, v0);
    const double d21 = DotProduct(v2, v1);
    const double denom = d00 * d11 - d01 * d01;

    if (fabs(denom) < 1e-12) {
        return Vector(1.0, 0.0, 0.0);
    }

    const double v = (d11 * d20 - d01 * d21) / denom;
    const double w = (d00 * d21 - d01 * d20) / denom;
    const double u = 1.0 - v - w;
    return Vector(u, v, w);
}

__host__ __device__ inline Vector GetNormal(const Triangle& tri, const Intersection& intersection) {
    if (!tri.HasVertexNormals()) {
        Vector n = CrossProduct(tri.B() - tri.A(), tri.C() - tri.A());
        n.Normalize();
        return n;
    }

    const Vector bary = GetBarycentricCoords(tri, intersection.GetPosition());
    Vector n = bary[0] * tri.NormalA() + bary[1] * tri.NormalB() + bary[2] * tri.NormalC();
    n.Normalize();
    return n;
}
