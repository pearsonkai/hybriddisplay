#include "Renderer.hpp"
#include <algorithm>
#include <cmath>

namespace hybriddisplay::rendering {

Renderer::Renderer()
{

}

Renderer::Renderer(threading::Pool* _pool) {
    pool = _pool;
}





void Renderer::putPixel(display::Viewport& viewport, int32_t localX, int32_t localY, const graphics::Colour& colour) {
    const uint32_t index = viewport.resolution().width * static_cast<uint32_t>(localY) + static_cast<uint32_t>(localX);
    viewport.framebuffer()->at(index) = colour;
}

void Renderer::putPixel(display::Viewport& viewport, uint32_t index, const graphics::Colour& colour) {
    viewport.framebuffer()->at(index) = colour;
}

void Renderer::drawLine(display::Viewport& viewport, const math::Vec3& p0, const math::Vec3& p1, const graphics::Colour& colour) {
    math::Vec3 clipped0 = p0;
    math::Vec3 clipped1 = p1;
    if (!clipLineToBounds(viewport, clipped0, clipped1))
        return;

    int x0 = static_cast<int>(std::lround(clipped0.x));
    int y0 = static_cast<int>(std::lround(clipped0.y));
    int x1 = static_cast<int>(std::lround(clipped1.x));
    int y1 = static_cast<int>(std::lround(clipped1.y));

    const int dx = std::abs(x1 - x0);
    const int dy = std::abs(y1 - y0);
    const int sx = (x0 < x1) ? 1 : -1;
    const int sy = (y0 < y1) ? 1 : -1;
    int err = dx - dy;

    while (true) {
        putPixel(viewport, x0, y0, colour);

        if (x0 == x1 && y0 == y1)
            break;

        const int e2 = err * 2;
        if (e2 > -dy) {
            err -= dy;
            x0 += sx;
        }
        if (e2 < dx) {
            err += dx;
            y0 += sy;
        }
    }
}

void Renderer::outlineViewport(display::Viewport& viewport, graphics::Colour colour)
{
    const int left = viewport.tileBounds.left;
    const int top = viewport.tileBounds.top;
    const int right = viewport.tileBounds.right - 1;
    const int bottom = viewport.tileBounds.bottom - 1;
    drawLine(viewport, math::Vec3(left, top), math::Vec3(right, top), colour);
    drawLine(viewport, math::Vec3(right, top), math::Vec3(right, bottom), colour);
    drawLine(viewport, math::Vec3(right, bottom), math::Vec3(left, bottom), colour);
    drawLine(viewport, math::Vec3(left, bottom), math::Vec3(left, top), colour);
}








void Renderer::transformBatchVertex(std::vector<geometry::Vertex>& list, const std::array<uint32_t, 2>& range, geometry::Mesh& mesh, const math::Transform& cameraTransform, const math::Transform& transform) {
    for(uint32_t i = range[0]; i < range[1]; i++) {
        geometry::Vertex newVertex = transform.apply(mesh.getVertex(i));
        
        newVertex.position = cameraTransform.applyInverseRotation(newVertex.position - cameraTransform.getPosition());
        newVertex.normal = cameraTransform.applyInverseRotation(newVertex.normal);
        
        list.at(i) = newVertex;
    }
}

void Renderer::transformBatchPosition(std::vector<math::Vec3>& list, const std::array<uint32_t, 2>& range, geometry::Mesh& mesh, const math::Transform& cameraTransform, const math::Transform& transform) {
    for(uint32_t i = range[0]; i < range[1]; i++) {
        math::Vec3 newVec3 = transform.applyPosition(mesh.getVertex(i).position);
        newVec3 = cameraTransform.applyInverseRotation(newVec3 - cameraTransform.getPosition());
        
        list.at(i) = newVec3;
    }
}


};