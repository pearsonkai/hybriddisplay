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
    const uint32_t index = viewport.resolution.width * static_cast<uint32_t>(localY) + static_cast<uint32_t>(localX);
    viewport.framebuffer->at(index) = colour;
}

void Renderer::putPixel(display::Viewport& viewport, uint32_t index, const graphics::Colour& colour) {
    viewport.framebuffer->at(index) = colour;
}




void Renderer::outlineViewport(display::Viewport& viewport)
{
    const int left = viewport.tileBounds.left;
    const int top = viewport.tileBounds.top;
    const int right = viewport.tileBounds.right - 1;
    const int bottom = viewport.tileBounds.bottom - 1;
    drawLine(viewport, math::Vec3(left, top), math::Vec3(right, top), graphics::COLOUR_RED);
    drawLine(viewport, math::Vec3(right, top), math::Vec3(right, bottom), graphics::COLOUR_RED);
    drawLine(viewport, math::Vec3(right, bottom), math::Vec3(left, bottom), graphics::COLOUR_RED);
    drawLine(viewport, math::Vec3(left, bottom), math::Vec3(left, top), graphics::COLOUR_RED);
}

void transformBatchVertex(std::vector<geometry::Vertex>& list, const std::array<uint32_t, 2>& range, geometry::Mesh& mesh, const math::Transform& cameraTransform, math::Transform& transform) {
    for(uint32_t i = range[0]; i < range[1]; i++) {
        geometry::Vertex newVertex = transform.apply(mesh.getVertex(i));
        
        newVertex.position = cameraTransform.applyInverseRotation(newVertex.position - cameraTransform.getPosition());
        newVertex.normal = cameraTransform.applyInverseRotation(newVertex.normal);
        
        list.at(i) = newVertex;
    }
}

};