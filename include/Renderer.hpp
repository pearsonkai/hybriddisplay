#ifndef RENDERER_HPP
#define RENDERER_HPP

#include "Screen.hpp"
#include "Pool.hpp"
#include "Camera.hpp"
#include "World.hpp"

namespace hybriddisplay::rendering {

class Renderer {
private:
    threading::Pool* pool;
public:
    
    Renderer();
    Renderer(threading::Pool* _pool);

    
    static const math::Vec3 project(const Camera& camera, const geometry::Vertex& vertex, const math::Transform& modelTransform, const display::Viewport& viewport);

    static void putPixel(display::Viewport& viewport, int localX, int localY, float depth, const graphics::Colour& colour);
    static void drawLine(display::Viewport& viewport, const math::Vec3& v0, const math::Vec3& v1, const graphics::Colour& colour = graphics::COLOUR_MAGENTA);

    void wireframe(display::Viewport& viewport, const Camera& camera, const geometry::World& world);
    void rasterize(display::Viewport& viewport, const Camera& camera, const geometry::World& world);
    void raytrace(display::Viewport& viewport, const Camera& camera, const geometry::World& world);
};

};

#endif