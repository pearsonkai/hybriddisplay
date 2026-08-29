#include "Transform.hpp"

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

Vec3 Transform::applyPosition(const Vec3& point) const {
    return position + rotation * (point * scale);
}

Vec3 Transform::applyNormal(const Vec3& normal) const {
    return rotation * (normal * scale);
}

};