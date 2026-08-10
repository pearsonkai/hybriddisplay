#ifndef VEC3_HPP
#define VEC3_HPP

namespace hybriddisplay::math {

class Vec3 {
public:
    float x, y, z;

    Vec3(float x = 0, float y = 0, float z = 0);

    Vec3 operator+(const Vec3& other) const;
    Vec3 operator-(const Vec3& other) const;
    Vec3 operator*(const Vec3& other) const;
    Vec3 operator/(const Vec3& other) const;

    Vec3& operator+=(const Vec3& other);
    Vec3& operator-=(const Vec3& other);
    Vec3& operator*=(const Vec3& other);
    Vec3& operator/=(const Vec3& other);

    Vec3 operator+(float scalar) const;
    Vec3 operator-(float scalar) const;
    Vec3 operator*(float scalar) const;
    Vec3 operator/(float scalar) const;

    float dot(const Vec3& other) const;
    Vec3 cross(const Vec3& other) const;
    Vec3 project(const Vec3& onto) const;
    Vec3 lerp(const Vec3& other, float t) const;
    Vec3 reflect(const Vec3& normal) const;

    float magnitude() const;
    Vec3 normalize() const;

    bool operator==(const Vec3& other) const;
    bool operator!=(const Vec3& other) const;
};

};

#endif