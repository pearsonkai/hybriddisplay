#include "Vec3.hpp"

namespace hybriddisplay::math {

    Vec3::Vec3(float x = 0, float y = 0, float z = 0) {
        this->x = x;
        this->y = y;
        this->z = z;
    }

    Vec3 Vec3::operator+(const Vec3& other) const {
        return Vec3(x + other.x, y + other.y, z + other.z);
    }
    Vec3 Vec3::operator-(const Vec3& other) const {
        return Vec3(x - other.x, y - other.y, z - other.z);
    }
    Vec3 Vec3::operator*(const Vec3& other) const {
        return Vec3(x * other.x, y * other.y, z * other.z);
    }
    Vec3 Vec3::operator/(const Vec3& other) const {
        return Vec3(x / other.x, y / other.y, z / other.z);
    }

    Vec3& Vec3::operator+=(const Vec3& other) {
        x += other.x;
        y += other.y;
        z += other.z;
        return *this;
    }
    Vec3& Vec3::operator-=(const Vec3& other) {
        x -= other.x;
        y -= other.y;
        z -= other.z;
        return *this;
    }
    Vec3& Vec3::operator*=(const Vec3& other) {
        x *= other.x;
        y *= other.y;
        z *= other.z;
        return *this;
    }
    Vec3& Vec3::operator/=(const Vec3& other) {
        x /= other.x;
        y /= other.y;
        z /= other.z;
        return *this;
    }

    Vec3 Vec3::operator+(float scalar) const {
        return Vec3(x + scalar, y + scalar, z + scalar);
    }
    Vec3 Vec3::operator-(float scalar) const {
        return Vec3(x - scalar, y - scalar, z - scalar);
    }
    Vec3 Vec3::operator*(float scalar) const {
        return Vec3(x * scalar, y * scalar, z * scalar);
    }
    Vec3 Vec3::operator/(float scalar) const {
        return Vec3(x / scalar, y / scalar, z / scalar);
    }

    float Vec3::dot(const Vec3& other) const {
        return x * other.x + y * other.y + z * other.z;
    }
    Vec3 Vec3::cross(const Vec3& other) const {
        return Vec3(
            y * other.z - z * other.y,
            z * other.x - x * other.z,
            x * other.y - y * other.x
        );
    }
    Vec3 Vec3::project(const Vec3& onto) const {
        float ontoMagnitudeSquared = onto.dot(onto);
        if (ontoMagnitudeSquared == 0) {
            return Vec3(0, 0, 0); // Avoid division by zero
        }
        return (dot(onto) / ontoMagnitudeSquared) * onto;
    }
    Vec3 Vec3::lerp(const Vec3& other, float t) const {
        return (*this) * (1 - t) + other * t;
    }
    Vec3 Vec3::reflect(const Vec3& normal) const {
        return (*this) - 2 * this->dot(normal) * normal;
    }

    float Vec3::magnitude() const {
        return std::sqrt(x * x + y * y + z * z);
    }
    Vec3 Vec3::normalize() const {
        float mag = magnitude();
        if (mag == 0) {
            return Vec3(0, 0, 0);
        }
        return (*this) / mag;
    }

    bool Vec3::operator==(const Vec3& other) const {
        return x == other.x && y == other.y && z == other.z;
    }
    bool Vec3::operator!=(const Vec3& other) const {
        return !(*this == other);
    }
};