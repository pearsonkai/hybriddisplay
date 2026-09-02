#include "Camera.hpp"
#include <cmath>

namespace hybriddisplay::rendering {

Camera::Camera()
{
    fov = 90.0f;
    aspectRatio = 16.0f / 9.0f;
    nearPlane = 0.1f;
    farPlane = 100.0f;
}

Camera::Camera(float _fov, float _aspectRatio, float _nearPlane, float _farPlane)
{
    fov = _fov;
    aspectRatio = _aspectRatio;
    nearPlane = _nearPlane;
    farPlane = _farPlane;
}

void Camera::pointTowards(const math::Vec3& point) {
    math::Vec3 direction = point - transform.getPosition();
    float length = std::sqrt(direction.x * direction.x + direction.y * direction.y + direction.z * direction.z);
    if (length > 0.0f) {
        direction = direction * (1.0f / length); // Normalize
        float pitch = std::asin(-direction.y);
        float yaw = std::atan2(direction.x, -direction.z);
        transform.setRotation(math::Vec3(pitch, yaw, 0.0f));
    }
}

void Camera::moveTowards(const math::Vec3& point, float distance) {
    math::Vec3 direction = point - transform.getPosition();
    float length = std::sqrt(direction.x * direction.x + direction.y * direction.y + direction.z * direction.z);
    if (length > 0.0f) {
        direction = direction * (1.0f / length); // Normalize
        transform.setPosition(transform.getPosition() + direction * distance);
    }
}

void Camera::goTo(const math::Vec3& point) {
    transform.setPosition(point);
}

math::Transform Camera::getTransform() const {
    return transform;
}



}