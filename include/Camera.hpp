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
    Camera(float fov, float aspectRatio, float nearPlane, float farPlane);

    math::Transform getTransform();

    void moveTowards(const math::Vec3& point, float distance);
    void pointTowards(const math::Vec3& point);
    void rotate(const math::Vec3& rotation);




    Vec3 project(const math::Vec3& vertex, const math::Transform& modelTransform, const display::Viewport& viewport);
    static void putPixel(display::Viewport& viewport, int localX, int localY, float depth, const rendering::Colour& colour)
    static void drawLine(display::Viewport& viewport, const geometry::Vertex& v1, const geometry::Vertex& v2);

    

    void wireframe(display::Viewport& viewport, const containers::World& world, threading::Pool& pool);
    void rasterize(display::Viewport& viewport, const containers::World& world, threading::Pool& pool);
    void raytrace(display::Viewport& viewport, const containers::World& world, threading::Pool& pool);

    //void Render(display::Viewport& viewport, const containers::World& world, threading::Pool& pool)
};

};

#endif