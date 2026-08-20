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

    
    static Vec3 project(const Camera& camera, const math::Vec3& vertex, const math::Transform& modelTransform, const display::Viewport& viewport);
    static void putPixel(display::Viewport& viewport, int localX, int localY, float depth, const graphics::Colour& colour)
    static void drawLine(display::Viewport& viewport, const geometry::Vertex& v1, const geometry::Vertex& v2);

    void wireframe(display::Viewport& viewport, const Camera& camera, const geometry::World& world);
    void rasterize(display::Viewport& viewport, const Camera& camera, const geometry::World& world);
    void raytrace(display::Viewport& viewport, const Camera& camera, const geometry::World& world);
};

};

#endif