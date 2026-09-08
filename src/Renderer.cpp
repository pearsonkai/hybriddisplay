#include "Renderer.hpp"
#include <algorithm>
#include <cmath>

namespace {

using hybriddisplay::math::Vec3;
using hybriddisplay::rendering::Camera;
using hybriddisplay::display::Viewport;
using hybriddisplay::graphics::Colour;
using hybriddisplay::rendering::Renderer;

Vec3 projectView(const Vec3& view, float areaWidth, float areaHeight)
{
    float x_ndc = view.x / -view.z;
    float y_ndc = view.y / -view.z;
    float x_screen = (x_ndc + 1.0f) * 0.5f * areaWidth;
    float y_screen = (1.0f - y_ndc) * 0.5f * areaHeight;
    return Vec3(x_screen, y_screen, -view.z);
}

void drawClippedLine(Viewport& viewport, float nearPlane, float areaWidth, float areaHeight, const Vec3& view0, const Vec3& view1, const Colour& colour)
{
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

    Renderer::drawLine(viewport, projectView(clipped0, areaWidth, areaHeight), projectView(clipped1, areaWidth, areaHeight), colour);
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

    return projectView(view, vp.area.width * vp.resolution.width, vp.area.height * vp.resolution.height);
}

void Renderer::putPixel(display::Viewport& viewport, int localX, int localY, float depth, const graphics::Colour& colour)
{
    const int areaWidth = static_cast<int>(std::lround(viewport.area.width * viewport.resolution.width));
    const int areaHeight = static_cast<int>(std::lround(viewport.area.height * viewport.resolution.height));
    const int tileLeft = static_cast<int>(std::floor(viewport.tile.x * areaWidth));
    const int tileTop = static_cast<int>(std::floor(viewport.tile.y * areaHeight));
    const int tileRight = static_cast<int>(std::ceil((viewport.tile.x + viewport.tile.width) * areaWidth));
    const int tileBottom = static_cast<int>(std::ceil((viewport.tile.y + viewport.tile.height) * areaHeight));

    if (localX < tileLeft || localX >= tileRight || localY < tileTop || localY >= tileBottom)
        return;

    const int areaLeft = static_cast<int>(std::lround(viewport.area.x * viewport.resolution.width));
    const int areaTop = static_cast<int>(std::lround(viewport.area.y * viewport.resolution.height));
    const int framebufferX = areaLeft + localX;
    const int framebufferY = areaTop + localY;
    const size_t index = static_cast<size_t>(framebufferY) * viewport.resolution.width + framebufferX;
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
    const float nearPlane = camera.getNearPlane();
    const math::Transform& cameraTransform = camera.getTransform();
    const math::Vec3& cameraPosition = cameraTransform.getPosition();

    for (const geometry::Model& model : world.getVisibleModels())
    {
        geometry::Mesh& mesh = *model.mesh;
        const math::Transform& modelTransform = model.transform;

        std::vector<math::Vec3> viewVertices;
        viewVertices.reserve(mesh.getNumVertices());

        for (uint32_t i = 0; i < mesh.getNumVertices(); ++i)
        {
            const math::Vec3 worldPosition =
                modelTransform.applyPosition(mesh.getVertex(i).position);

            viewVertices.emplace_back(
                cameraTransform.applyInverseRotation(worldPosition - cameraPosition)
            );
        }

        for (display::Viewport& viewport : viewports)
        {
            const float areaWidth = viewport.area.width * viewport.resolution.width;

            const float areaHeight = viewport.area.height * viewport.resolution.height;

            const float tileLeft = viewport.tile.x;
            const float tileTop = viewport.tile.y;
            const float tileRight = tileLeft + viewport.tile.width;
            const float tileBottom = tileTop + viewport.tile.height;

            for (uint32_t i = 0; i < mesh.getNumFaces(); ++i)
            {
                //const auto indices = mesh.getTriIndices(i);

                const uint32_t i0 = mesh.getIndice(i * 3 + 0);
                const uint32_t i1 = mesh.getIndice(i * 3 + 1);
                const uint32_t i2 = mesh.getIndice(i * 3 + 2);

                const math::Vec3& a = viewVertices[i0];
                const math::Vec3& b = viewVertices[i1];
                const math::Vec3& c = viewVertices[i2];

                const math::Vec3 pa = projectView(a, areaWidth, areaHeight);
                const math::Vec3 pb = projectView(b, areaWidth, areaHeight);
                const math::Vec3 pc = projectView(c, areaWidth, areaHeight);

                const float left = std::min({pa.x / areaWidth, pb.x / areaWidth, pc.x / areaWidth});
                const float right = std::max({pa.x / areaWidth, pb.x / areaWidth, pc.x / areaWidth});
                const float top = std::min({pa.y / areaHeight, pb.y / areaHeight, pc.y / areaHeight});
                const float bottom = std::max({pa.y / areaHeight, pb.y / areaHeight, pc.y / areaHeight});

                if (-a.z >= nearPlane && -b.z >= nearPlane && -c.z >= nearPlane &&
                    (right < tileLeft || left > tileRight ||
                     bottom < tileTop || top > tileBottom))
                {
                    continue;
                }

                drawClippedLine(viewport, nearPlane, areaWidth, areaHeight, a, b, graphics::COLOUR_MAGENTA);

                drawClippedLine(viewport, nearPlane, areaWidth, areaHeight, b, c, graphics::COLOUR_MAGENTA);

                drawClippedLine(viewport, nearPlane, areaWidth, areaHeight, c, a, graphics::COLOUR_MAGENTA);
            }
        }
    }
}

};