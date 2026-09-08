#include "Renderer.hpp"
#include <algorithm>
#include <cmath>

namespace {

using hybriddisplay::math::Vec3;
using hybriddisplay::rendering::Camera;
using hybriddisplay::display::Viewport;
using hybriddisplay::graphics::Colour;
using hybriddisplay::rendering::Renderer;

Vec3 projectView(const Vec3& view, const Viewport& viewport)
{
    float x_ndc = view.x / -view.z;
    float y_ndc = view.y / -view.z;
    float areaWidth = viewport.area.width * viewport.resolution.width;
    float areaHeight = viewport.area.height * viewport.resolution.height;
    float x_screen = (x_ndc + 1.0f) * 0.5f * areaWidth;
    float y_screen = (1.0f - y_ndc) * 0.5f * areaHeight;
    return Vec3(x_screen, y_screen, -view.z);
}

void drawClippedLine(Viewport& viewport, const Camera& camera, const Vec3& view0, const Vec3& view1, const Colour& colour)
{
    const float nearPlane = std::max(camera.getNearPlane(), 0.0001f);
    const float depth0 = -view0.z;
    const float depth1 = -view1.z;

    if (depth0 < nearPlane && depth1 < nearPlane) return;

    Vec3 clipped0 = view0;
    Vec3 clipped1 = view1;
    if (depth0 < nearPlane) {
        const float amount = (nearPlane - depth0) / (depth1 - depth0);
        clipped0 = view0 + (view1 - view0) * amount;
    }
    if (depth1 < nearPlane) {
        const float amount = (nearPlane - depth0) / (depth1 - depth0);
        clipped1 = view0 + (view1 - view0) * amount;
    }

    Renderer::drawLine(viewport, projectView(clipped0, viewport), projectView(clipped1, viewport), colour);
}

}

namespace hybriddisplay::rendering {

Renderer::Renderer()
{

}

Renderer::Renderer(threading::Pool* _pool) {
    pool = _pool;
} 

const math::Vec3 Renderer::project(const Camera& camera, const geometry::Vertex& v, const math::Transform& t, const display::Viewport& vp)
{
    math::Vec3 world = t.applyPosition(v.position);
    
    math::Vec3 view = world - camera.getTransform().getPosition();
    view = camera.getTransform().applyInverseRotation(view);

    return projectView(view, vp);
}

void Renderer::putPixel(display::Viewport& viewport, int localX, int localY, float depth, const graphics::Colour& colour)
{
    if (localX < 0 || localX >= viewport.area.width * viewport.resolution.width || localY < 0 || localY >= viewport.area.height * viewport.resolution.height)
        return;

    const size_t index = static_cast<size_t>(localY) * viewport.resolution.width + localX;
    auto& framebuffer = *viewport.framebuffer;
    auto& zbuffer = *viewport.zbuffer;

    if (depth < zbuffer[index]) {
        zbuffer[index] = depth;
        framebuffer[index] = colour;
    }
}

void Renderer::drawLine(display::Viewport& viewport, const math::Vec3& p0, const math::Vec3& p1, const graphics::Colour &colour)
{
    float dx = p1.x - p0.x;
    float dy = p1.y - p0.y;
    float steps = std::max(std::abs(dx), std::abs(dy));
    if (steps <= 0.0f) {
        int lx = static_cast<int>(std::lround(p0.x));
        int ly = static_cast<int>(std::lround(p0.y));
        putPixel(viewport, lx, ly, p0.z, colour);
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
        putPixel(viewport, localX, localY, z, colour);
        x += ix;
        y += iy;
        z += iz;
    }
}


void Renderer::outlineViewport(display::Viewport& viewport)
{
    const int width = static_cast<int>(std::lround(viewport.area.width * viewport.resolution.width));
    const int height = static_cast<int>(std::lround(viewport.area.height * viewport.resolution.height));
    const int left = static_cast<int>(std::floor(viewport.tile.x * width));
    const int top = static_cast<int>(std::floor(viewport.tile.y * height));
    const int right = static_cast<int>(std::ceil((viewport.tile.x + viewport.tile.width) * width)) - 1;
    const int bottom = static_cast<int>(std::ceil((viewport.tile.y + viewport.tile.height) * height)) - 1;
    drawLine(viewport, math::Vec3(left, top), math::Vec3(right, top), graphics::COLOUR_RED);
    drawLine(viewport, math::Vec3(right, top), math::Vec3(right, bottom), graphics::COLOUR_RED);
    drawLine(viewport, math::Vec3(right, bottom), math::Vec3(left, bottom), graphics::COLOUR_RED);
    drawLine(viewport, math::Vec3(left, bottom), math::Vec3(left, top), graphics::COLOUR_RED);
}

void Renderer::wireframe(std::vector<display::Viewport>& viewports, const Camera& camera, const geometry::World& world)
{

    for (const geometry::Model& model : world.getVisibleModels())
    {
        geometry::Mesh& mesh = (*model.mesh);
        math::Transform modelTransform = model.transform;

        std::vector<math::Vec3> transformedVertices;

        for (uint32_t i = 0; i < mesh.getNumVertices(); ++i) 
        {
            geometry::Vertex vertex = mesh.getVertex(i);
            math::Vec3 worldPosition = modelTransform.applyPosition(vertex.position);
            transformedVertices.push_back(worldPosition);
        }


        for (uint32_t i = 0; i < mesh.getNumFaces(); ++i) 
        {
            geometry::Triangle triangle = mesh.getTri(i);

            const math::Vec3 aWorld = modelTransform.applyPosition(triangle.v0->position);
            const math::Vec3 bWorld = modelTransform.applyPosition(triangle.v1->position);
            const math::Vec3 cWorld = modelTransform.applyPosition(triangle.v2->position);

            for (display::Viewport& viewport : viewports)
            {
                drawClippedLine(viewport, camera, aWorld, bWorld, graphics::COLOUR_MAGENTA);
                drawClippedLine(viewport, camera, bWorld, cWorld, graphics::COLOUR_MAGENTA);
                drawClippedLine(viewport, camera, cWorld, aWorld, graphics::COLOUR_MAGENTA);
            }
        }
    }
    /*
    const math::Transform cameraTransform = camera.getTransform();
    const math::Vec3 cameraPosition = cameraTransform.getPosition();
    const float nearPlane = std::max(camera.getNearPlane(), 0.0001f);

    const float areaWidth = viewport.area.width * viewport.resolution.width;
    const float areaHeight = viewport.area.height * viewport.resolution.height;
    const float tileLeft = viewport.tile.x * areaWidth;
    const float tileTop = viewport.tile.y * areaHeight;
    const float tileRight = (viewport.tile.x + viewport.tile.width) * areaWidth;
    const float tileBottom = (viewport.tile.y + viewport.tile.height) * areaHeight;

    for (const geometry::Model& model : world.getVisibleModels())
    {
        geometry::Mesh& mesh = (*model.mesh);
        math::Transform modelTransform = model.transform;

        for (uint32_t i = 0; i < mesh.getNumFaces(); ++i) 
        {
            geometry::Triangle triangle = mesh.getTri(i);

            const math::Vec3 aWorld = modelTransform.applyPosition(triangle.v0->position);
            const math::Vec3 bWorld = modelTransform.applyPosition(triangle.v1->position);
            const math::Vec3 cWorld = modelTransform.applyPosition(triangle.v2->position);
            const math::Vec3 a = cameraTransform.applyInverseRotation(aWorld - cameraPosition);
            const math::Vec3 b = cameraTransform.applyInverseRotation(bWorld - cameraPosition);
            const math::Vec3 c = cameraTransform.applyInverseRotation(cWorld - cameraPosition);
            
            if (-a.z >= nearPlane && -b.z >= nearPlane && -c.z >= nearPlane) {
                const math::Vec3 screenA = projectView(a, viewport);
                const math::Vec3 screenB = projectView(b, viewport);
                const math::Vec3 screenC = projectView(c, viewport);

                const float triangleLeft = std::min({screenA.x, screenB.x, screenC.x});
                const float triangleTop = std::min({screenA.y, screenB.y, screenC.y});
                const float triangleRight = std::max({screenA.x, screenB.x, screenC.x});
                const float triangleBottom = std::max({screenA.y, screenB.y, screenC.y});

                if (triangleRight < tileLeft || triangleLeft > tileRight ||
                    triangleBottom < tileTop || triangleTop > tileBottom)
                {
                    continue;
                }
            } // out of tile culling

            drawClippedLine(viewport, camera, a, b, graphics::COLOUR_MAGENTA);
            drawClippedLine(viewport, camera, b, c, graphics::COLOUR_MAGENTA);
            drawClippedLine(viewport, camera, c, a, graphics::COLOUR_MAGENTA);
        }
    }
        */
}




};