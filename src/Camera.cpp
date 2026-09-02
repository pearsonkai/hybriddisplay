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

math::Transform Camera::getTransform() const {
    return transform;
}



}