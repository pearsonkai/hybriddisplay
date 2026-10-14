#include "Camera.hpp"
#include <algorithm>
#include <cmath>

namespace hybriddisplay::rendering {

Camera::Camera()
{
    setFov(90.0f);
    nearPlane = 0.001f;
    farPlane = 100.0f;
}

Camera::Camera(float _fov, float _nearPlane, float _farPlane)
{
    setFov(_fov);
    nearPlane = _nearPlane;
    farPlane = _farPlane;
}










const math::Transform& Camera::getTransform() const {
    return transform;
}

float Camera::getNearPlane() const {
    return nearPlane;
}


float Camera::getFarPlane() const {
    return farPlane;
}

float Camera::getFov() const {
    return fov;
}

float Camera::getFocalLength() const {
    return fval;
}
    










void Camera::setNearPlane(float distance) {
    nearPlane = distance;
}

void Camera::setFarPlane(float distance) {
    farPlane = distance;
}

void Camera::setFov(const float degrees) {
    fov = degrees;
    const float radians = degrees * 3.14159265359f / 180.0f;
    fval = 1.0f / std::tan(radians * 0.5f);
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

void Camera::moveForward(float distance) {
    const math::Vec3 forward = transform.applyRotation(math::Vec3(0, 0, -1));
    moveTowards(transform.getPosition() + forward, distance);
}

void Camera::moveUp(float distance) {
    const math::Vec3 up = transform.applyRotation(math::Vec3(0, 1, 0));
    moveTowards(transform.getPosition() + up, distance);
}

void Camera::moveRight(float distance) {
    const math::Vec3 right = transform.applyRotation(math::Vec3(1, 0, 0));
    moveTowards(transform.getPosition() + right, distance);    
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

void Camera::rotate(const math::Vec3& rotation) {
    transform.rotate(rotation);
}

void Camera::rotate(const float pitch, const float yaw) {
    rotatePitch(pitch);
    rotateYaw(yaw);
}

void Camera::rotatePitch(const float degrees) {
    constexpr float radiansToDegrees = 180.0f / 3.14159265359f;
    constexpr float maxPitch = 89.0f / radiansToDegrees;

    const float currentPitch = transform.getRotation().x;
    const float targetPitch = std::clamp(
        currentPitch + degrees / radiansToDegrees,
        -maxPitch,
        maxPitch
    );
    transform.rotatePitch((targetPitch - currentPitch) * radiansToDegrees);
}

void Camera::rotateYaw(const float degrees) {
    transform.rotateYaw(degrees);
}











math::Vec3 Camera::projectView(const math::Vec3& view, float width, float height) const {
    const float depth = -view.z;
    const float aspect = 1;//swidth / height;

    const float x_ndc = view.x / depth * fval / aspect;
    const float y_ndc = view.y / depth * fval;

    const float x_screen = (x_ndc + 1.0f) * 0.5f * width;
    const float y_screen = (1.0f - y_ndc) * 0.5f * height;

    return math::Vec3(x_screen, y_screen, depth);
}

}