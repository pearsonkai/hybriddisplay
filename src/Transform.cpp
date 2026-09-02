#include "Transform.hpp"
#include <cmath>

namespace hybriddisplay::math {

Transform::Transform(const Vec3& position, const Vec3& rotation, const Vec3& scale)
    : position(position), rotation(rotation), scale(scale) {
}

void Transform::setPosition(const Vec3& position) {
    this->position = position;
}

void Transform::setRotation(const Vec3& rotation) {
    this->rotation = rotation;
}

void Transform::setScale(const Vec3& scale) {
    this->scale = scale;
}

const Vec3& Transform::getPosition() const {
    return position;
}

const Vec3& Transform::getRotation() const {
    return rotation;
}

const Vec3& Transform::getScale() const {
    return scale;
}

Vec3 Transform::applyRotation(const Vec3& point) const {
    const float sinX = std::sin(rotation.x);
    const float cosX = std::cos(rotation.x);
    const float sinY = std::sin(rotation.y);
    const float cosY = std::cos(rotation.y);
    const float sinZ = std::sin(rotation.z);
    const float cosZ = std::cos(rotation.z);

    Vec3 rotated = point;
    rotated = Vec3(rotated.x,
                   rotated.y * cosX - rotated.z * sinX,
                   rotated.y * sinX + rotated.z * cosX);
    rotated = Vec3(rotated.x * cosY + rotated.z * sinY,
                   rotated.y,
                   -rotated.x * sinY + rotated.z * cosY);
    return Vec3(rotated.x * cosZ - rotated.y * sinZ,
                rotated.x * sinZ + rotated.y * cosZ,
                rotated.z);
}

Vec3 Transform::applyInverseRotation(const Vec3& point) const {
    Transform inverse(Vec3(0, 0, 0), rotation * -1.0f, Vec3(1, 1, 1));
    return inverse.applyRotation(point);
}

Vec3 Transform::applyPosition(const Vec3& point) const {
    return position + applyRotation(point * scale);
}

Vec3 Transform::applyNormal(const Vec3& normal) const {
    return applyRotation(normal * scale);
}

};