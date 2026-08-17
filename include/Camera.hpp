#ifndef CAMERA_HPP
#define CAMERA_HPP

#include "World.hpp"
#include "Screen.hpp"
#include "Pool.hpp"

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

    void Wireframe(display::Viewport& viewport, const containers::World& world, threading::Pool& pool);
    void Rasterize(display::Viewport& viewport, const containers::World& world, threading::Pool& pool);
    void Raytrace(display::Viewport& viewport, const containers::World& world, threading::Pool& pool);

    void Render(display::Viewport& viewport, const containers::World& world, threading::Pool& pool)
};

};

#endif