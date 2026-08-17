#ifndef CAMERA_HPP
#define CAMERA_HPP

#include "World.hpp"

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

    
};

};

#endif