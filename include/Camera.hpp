#ifndef CAMERA_HPP
#define CAMERA_HPP

#include "Transform.hpp"

namespace hybriddisplay::rendering {

class Camera {
private:
    math::Transform transform;
    float fov; // field of view in degrees
    float fval;

    float nearPlane; // near clipping plane
    float farPlane; // far clipping plane
public:
    
    Camera();
    Camera(float fov, float nearPlane, float farPlane);

    const math::Transform& getTransform() const;
    float getNearPlane() const;

    void setFov(const float degrees);
    void moveTowards(const math::Vec3& point, float distance);
    void goTo(const math::Vec3& point);
    void pointTowards(const math::Vec3& point);
    void rotate(const math::Vec3& rotation);

    math::Vec3 projectView(const math::Vec3& view, float width, float height) const;
};

};

#endif