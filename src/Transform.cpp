#include "Transform.hpp"
#include <cmath>

namespace hybriddisplay::math {

Transform::Transform(const Vec3& position, const Vec3& rotation, const Vec3& scale)
    : position(position), rotation(rotation), scale(scale) 
{
        updateCache();
}

void Transform::setPosition(const Vec3& position) {
    this->position = position;
}

void Transform::setRotation(const Vec3& rotation) {
    this->rotation = rotation;
    updateCache();
}

void Transform::setScale(const Vec3& scale) {
    this->scale = scale;
}

void Transform::rotate(const Vec3& rotation) {
    constexpr float degreesToRadians = 3.14159265359f / 180.0f;
    setRotation(this->rotation + rotation * degreesToRadians);
}

void Transform::rotate(const float pitch, const float yaw) {
    rotatePitch(pitch);
    rotateYaw(yaw);
}

void Transform::rotatePitch(const float degrees) {
    rotate(Vec3(degrees, 0.0f, 0.0f));
}

void Transform::rotateYaw(const float degrees) {
    rotate(Vec3(0.0f, degrees, 0.0f));
}

void Transform::rotateRoll(const float degrees) {
    rotate(Vec3(0.0f, 0.0f, degrees));
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





void Transform::updateCache() {
    x.sin = std::sin(rotation.x);
    x.cos = std::cos(rotation.x);
    y.sin = std::sin(rotation.y);
    y.cos = std::cos(rotation.y);
    z.sin = std::sin(rotation.z);
    z.cos = std::cos(rotation.z);
}

Vec3 Transform::applyRotation(const Vec3& point) const {
    Vec3 rotated = point;
    
    rotated = Vec3(rotated.x, rotated.y * x.cos - rotated.z * x.sin, rotated.y * x.sin + rotated.z * x.cos);
    rotated = Vec3(rotated.x * y.cos + rotated.z * y.sin, rotated.y, -rotated.x * y.sin + rotated.z * y.cos);
    return Vec3(rotated.x * z.cos - rotated.y * z.sin, rotated.x * z.sin + rotated.y * z.cos, rotated.z);
}

Vec3 Transform::applyInverseRotation(const Vec3& point) const {
    Vec3 rotated = point;

    rotated = Vec3(
        rotated.x * z.cos + rotated.y * z.sin,
       -rotated.x * z.sin + rotated.y * z.cos,
        rotated.z
    );

    rotated = Vec3(
        rotated.x * y.cos - rotated.z * y.sin,
        rotated.y,
        rotated.x * y.sin + rotated.z * y.cos
    );

    rotated = Vec3(
        rotated.x,
        rotated.y * x.cos + rotated.z * x.sin,
       -rotated.y * x.sin + rotated.z * x.cos
    );

    return rotated;
}

Vec3 Transform::applyPosition(const Vec3& point) const {
    return position + applyRotation(point * scale);
}

Vec3 Transform::applyNormal(const Vec3& normal) const {
    return applyRotation(normal * scale);
}

geometry::Vertex Transform::apply(const geometry::Vertex& vertex) {
    return {applyPosition(vertex.position),applyNormal(vertex.normal),vertex.uv};
}

};