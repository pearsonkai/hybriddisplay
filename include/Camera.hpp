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
    float getFarPlane() const;
    float getFov() const;
    float getFocalLength() const;
    
    void setNearPlane(float distance);
    void setFarPlane(float distance);
    void setFov(const float degrees);


    void moveTowards(const math::Vec3& point, float distance);
    
    void moveForward(float distance);
    void moveUp(float distance);
    void moveRight(float distance);
    void goTo(const math::Vec3& point);


    void pointTowards(const math::Vec3& point);
    void rotate(const math::Vec3& rotation);
    void rotate(const float pitch, const float yaw);
    void rotatePitch(const float degrees);
    void rotateYaw(const float degrees);
    
    math::Vec3 projectView(const math::Vec3& view, float width, float height) const;
};

};

#endif