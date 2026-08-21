#ifndef CAMERA_HPP
#define CAMERA_HPP

#include "Transform.hpp"

namespace hybriddisplay::rendering {

class Camera {
private:
    math::Transform transform;
    float fov; // field of view in degrees
    float aspectRatio; // width / height
    float nearPlane; // near clipping plane
    float farPlane; // far clipping plane
public:
    
    Camera();
    Camera(float fov, float aspectRatio, float nearPlane, float farPlane);

    math::Transform getTransform() const;

    void moveTowards(const math::Vec3& point, float distance);
    void pointTowards(const math::Vec3& point);
    void rotate(const math::Vec3& rotation);
};

};

#endif