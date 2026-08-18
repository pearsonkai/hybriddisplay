#include "Camera.hpp"

namespace hybriddisplay::rendering {

Camera::Camera() 
    : fov(90.0f), aspectRatio(16.0f / 9.0f), nearPlane(0.1f), farPlane(100.0f) {}

Camera::Camera(float fov, float aspectRatio, float nearPlane, float farPlane)
    : fov(fov), aspectRatio(aspectRatio), nearPlane(nearPlane), farPlane(farPlane) {}



Vec3 Camera::project(const geometry::Vertex& v, const Transform& t, const Viewport& vp)
{
    // Object -> World
    math::Vec3 world =
        t.position +
        t.rotation * (v.position * t.scale);

    // World -> Camera
    math::Vec3 view = world - transform.position;
    view = inverse(transform.rotation) * view;

    // Perspective projection
    float x_ndc = view.x / -view.z;
    float y_ndc = view.y / -view.z;

    // Screen coordinates
    float x_screen =
        (x_ndc + 1.0f) * 0.5f * vp.area.width;

    float y_screen =
        (1.0f - y_ndc) * 0.5f * vp.area.height;

    return math::Vec3(x_screen, y_screen, -view.z);
}


void drawLine(const geometry::Vertex& v1, const geometry::Vertex& v2, const graphics::Colour& colour, display::Viewport& viewport)
{
    
}

    
void Camera::wireframe(display::Viewport& viewport, const containers::World& world, threading::Pool& pool)
{
    for (auto& model : world.getVisibleModels())
    {
        Mesh* mesh = model.mesh;
        
        for (const Triangle& triangle : mesh->triangles)
        {
            math::Vec3 a = project(mesh->getVertex(triangle.v1),model.transform,viewport);
            math::Vec3 b = project(mesh->getVertex(triangle.v2),model.transform,viewport);
            math::Vec3 c = project(mesh->getVertex(triangle.v3),model.transform,viewport);

            pool->addTask([&](){ drawLine(a, b, viewport); });
            pool->addTask([&](){ drawLine(b, c, viewport); });
            pool->addTask([&](){ drawLine(c, a, viewport); });
        }
    }
    
}


}