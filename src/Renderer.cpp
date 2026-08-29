#include "Renderer.hpp"

namespace hybriddisplay::rendering {

Renderer::Renderer()
{

}

Renderer::Renderer(threading::Pool* _pool) {
    pool = _pool;
} 

const math::Vec3 Renderer::project(const Camera& camera, const geometry::Vertex& v, const math::Transform& t, const display::Viewport& vp)
{
    math::Vec3 world = t.getPosition() + t.getRotation() * (v.position * t.getScale());
    
    math::Vec3 view = world - camera.getTransform().getPosition();
    view = camera.getTransform().getRotation().inverse() * view;

    float x_ndc = view.x / -view.z;
    float y_ndc = view.y / -view.z;

    float x_screen = (x_ndc + 1.0f) * 0.5f * vp.area.width;

    float y_screen = (1.0f - y_ndc) * 0.5f * vp.area.height;

    return math::Vec3(x_screen, y_screen, -view.z); // <--- switch view.z and -view.z to determine whether z is out or in space 
}

void Renderer::putPixel(display::Viewport& viewport, int localX, int localY, float depth, const graphics::Colour& colour)
{
    if (!viewport.framebuffer || !viewport.zbuffer) return;

    uint32_t w = viewport.area.width;
    uint32_t h = viewport.area.height;
    if (localX < 0 || localX >= static_cast<int>(w) || localY < 0 || localY >= static_cast<int>(h)) return;

    size_t idx = static_cast<size_t>(localY) * w + static_cast<size_t>(localX);
    auto &zb = *viewport.zbuffer;
    auto &fb = *viewport.framebuffer;
    if (idx >= zb.size() || idx >= fb.size()) return;

    if (depth < zb[idx]) {
        zb[idx] = depth;
        fb[idx] = colour;
    }
}

void Renderer::drawLine(display::Viewport& viewport, const math::Vec3& p0, const math::Vec3& p1)
{
    float dx = p1.x - p0.x;
    float dy = p1.y - p0.y;
    float steps = std::max(std::abs(dx), std::abs(dy));
    if (steps <= 0.0f) {
        int lx = static_cast<int>(std::lround(p0.x));
        int ly = static_cast<int>(std::lround(p0.y));
        putPixel(viewport, lx, ly, p0.z, graphics::COLOUR_MAGENTA);
        return;
    }

    float ix = dx / steps;
    float iy = dy / steps;
    float iz = (p1.z - p0.z) / steps;

    float x = p0.x;
    float y = p0.y;
    float z = p0.z;
    for (int i = 0; i <= static_cast<int>(steps); ++i) 
    {
        int localX = round(x);
        int localY = round(y);
        putPixel(viewport, localX, localY, z, graphics::COLOUR_MAGENTA);
        x += ix;
        y += iy;
        z += iz;
    }
}

void Renderer::wireframe(display::Viewport& viewport, const Camera& camera, const geometry::World& world)
{
    for (const geometry::Model& model : world.getVisibleModels())
    {
        geometry::Mesh* mesh = model.mesh;
        math::Transform modelTransform = model.transform;

        for (geometry::Triangle& triangle : mesh->getAllTri())
        {
            math::Vec3 a = project(camera, *triangle.v0, modelTransform, viewport);
            math::Vec3 b = project(camera, *triangle.v1, modelTransform, viewport);
            math::Vec3 c = project(camera, *triangle.v2, modelTransform, viewport);

            drawLine(viewport, a, b);
            drawLine(viewport, b, c);
            drawLine(viewport, c, a);
            /*
            pool->addTask([this, a, b, &viewport]() { drawLine(viewport, a, b); });
            pool->addTask([this, b, c, &viewport]() { drawLine(viewport, b, c); });
            pool->addTask([this, c, a, &viewport]() { drawLine(viewport, c, a); });
            */
        }
    }
    
}




};