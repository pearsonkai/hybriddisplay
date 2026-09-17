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

    static void putPixel(display::Viewport& viewport, int32_t localX, int32_t localY, const graphics::Colour& colour);
    static void putPixel(display::Viewport& viewport, uint32_t index, const graphics::Colour& colour);

    static bool clipLineToBounds(const display::Viewport& viewport, math::Vec3& p0, math::Vec3& p1);
    static void drawLine(display::Viewport& viewport, const math::Vec3& v0, const math::Vec3& v1, const graphics::Colour& colour = graphics::COLOUR_MAGENTA);
    static void outlineViewport(display::Viewport& viewport, graphics::Colour colour = graphics::COLOUR_RED);
    
    static void Renderer::transformBatchVertex(std::vector<geometry::Vertex>& list, const std::array<uint32_t, 2>& range, geometry::Mesh& mesh, const math::Transform& cameraTransform, math::Transform& transform);
    static void Renderer::transformBatchPosition(std::vector<math::Vec3>& list, const std::array<uint32_t, 2>& range, geometry::Mesh& mesh, const math::Transform& cameraTransform, const math::Transform& transform);

    void wireframe(std::vector<display::Viewport>& viewports, const Camera& camera, const geometry::World& world);
    void rasterize(std::vector<display::Viewport>& viewports, const Camera& camera, const geometry::World& world);
    void raytrace(std::vector<display::Viewport>& viewports, const Camera& camera, const geometry::World& world);
};

};

#endif