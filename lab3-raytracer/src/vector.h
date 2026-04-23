#pragma once

#include <cmath>

class Vector;
double Length(const Vector& v);

class Vector {
public:
    Vector() : x_(0.0), y_(0.0), z_(0.0) {
    }

    Vector(double x, double y, double z) : x_(x), y_(y), z_(z) {
    }

    double operator[](int i) const {
        if (i == 0) {
            return x_;
        }
        if (i == 1) {
            return y_;
        }
        return z_;
    }

    double& operator[](int i) {
        if (i == 0) {
            return x_;
        }
        if (i == 1) {
            return y_;
        }
        return z_;
    }

    Vector operator+(const Vector& rhs) const {
        return {x_ + rhs.x_, y_ + rhs.y_, z_ + rhs.z_};
    }

    Vector operator-(const Vector& rhs) const {
        return {x_ - rhs.x_, y_ - rhs.y_, z_ - rhs.z_};
    }

    Vector operator*(double k) const {
        return {x_ * k, y_ * k, z_ * k};
    }

    Vector operator/(double k) const {
        return {x_ / k, y_ / k, z_ / k};
    }

    Vector& operator+=(const Vector& rhs) {
        x_ += rhs.x_;
        y_ += rhs.y_;
        z_ += rhs.z_;
        return *this;
    }

    void Normalize() {
        const double len = Length(*this);
        if (len < 1e-12) {
            return;
        }
        x_ /= len;
        y_ /= len;
        z_ /= len;
    }

private:
    double x_;
    double y_;
    double z_;
};

inline Vector operator*(double k, const Vector& v) {
    return v * k;
}

inline double DotProduct(const Vector& a, const Vector& b) {
    return a[0] * b[0] + a[1] * b[1] + a[2] * b[2];
}

inline Vector CrossProduct(const Vector& a, const Vector& b) {
    return {a[1] * b[2] - a[2] * b[1], a[2] * b[0] - a[0] * b[2], a[0] * b[1] - a[1] * b[0]};
}

inline double Length(const Vector& v) {
    return std::sqrt(DotProduct(v, v));
}