#include "Camera.hpp"
#include <cmath>

namespace hybriddisplay::rendering {

Camera::Camera()
{
    setFov(90.0f);
    nearPlane = 0.1f;
    farPlane = 100.0f;
}

Camera::Camera(float _fov, float _nearPlane, float _farPlane)
{
    setFov(_fov);
    nearPlane = _nearPlane;
    farPlane = _farPlane;
}

void Camera::setFov(const float degrees) {
    fov = degrees;
    const float radians = degrees * 3.14159265359f / 180.0f;
    fval = 1.0f / std::tan(radians * 0.5f);
}

void Camera::pointTowards(const math::Vec3& point) {
    math::Vec3 direction = point - transform.getPosition();
    float length = std::sqrt(direction.x * direction.x + direction.y * direction.y + direction.z * direction.z);
    if (length > 0.0f) {
        direction = direction * (1.0f / length); // Normalize
        float pitch = std::asin(direction.y);
        float yaw = std::atan2(-direction.x, -direction.z);
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

const math::Transform& Camera::getTransform() const {
    return transform;
}

float Camera::getNearPlane() const {
    return nearPlane;
}


math::Vec3 Camera::projectView(const math::Vec3& view, float width, float height) const {
    const float x_ndc = (view.x / -view.z) * fval;
    const float y_ndc = (view.y / -view.z) * fval;

    const float x_screen = (x_ndc + 1.0f) * 0.5f * width;
    const float y_screen = (1.0f - y_ndc) * 0.5f * height;

    return math::Vec3(x_screen, y_screen, -view.z);
}

}