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

void drawClippedLine(Viewport& viewport, float nearPlane, float areaWidth, float areaHeight, const Vec3& view0, const Vec3& view1, const Colour& colour) {
    float depth0 = -view0.z;
    float depth1 = -view1.z;

    // Entirely behind near plane
    if (depth0 < nearPlane && depth1 < nearPlane)
        return;

    Vec3 a = view0;
    Vec3 b = view1;

    // Clip A against near plane
    if (depth0 < nearPlane) {
        float t = (nearPlane - depth0) / (depth1 - depth0);
        a = view0 + (view1 - view0) * t;
    }

    // Clip B against near plane
    if (depth1 < nearPlane) {
        float t = (nearPlane - depth0) / (depth1 - depth0);
        b = view0 + (view1 - view0) * t;
    }

    Renderer::drawLine(viewport, projectView(a, areaWidth, areaHeight), projectView(b, areaWidth, areaHeight), colour);
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

void Renderer::putPixel(display::Viewport& viewport, uint32_t localX, uint32_t localY, const graphics::Colour& colour) {
    if (localX < viewport.tileBounds.left || localX >= viewport.tileBounds.right ||
        localY < viewport.tileBounds.top || localY >= viewport.tileBounds.bottom)
        return;
    
    const uint32_t index = viewport.resolution.width * localY + localX;
    viewport.framebuffer->at(index) = colour;
}

void Renderer::putPixel(display::Viewport& viewport, uint32_t index, const graphics::Colour& colour) {
    viewport.framebuffer->at(index) = colour;
}

void Renderer::drawLine(Viewport& viewport, const Vec3& p0, const Vec3& p1, const Colour& colour) {
    float dx = p1.x - p0.x;
    float dy = p1.y - p0.y;

    float steps = std::max(std::abs(dx), std::abs(dy));

    if (steps <= 0.0f) {
        putPixel(viewport,static_cast<uint32_t>(std::lround(p0.x)),static_cast<uint32_t>(std::lround(p0.y)),colour);
        return;
    }

    float ix = dx / steps;
    float iy = dy / steps;

    float x = p0.x;
    float y = p0.y;

    for (int i = 0; i <= static_cast<int>(steps); ++i)
    {
        putPixel(
            viewport,
            static_cast<uint32_t>(std::lround(x)),
            static_cast<uint32_t>(std::lround(y)),
            colour);

        x += ix;
        y += iy;
    }
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

void transformBatchPosition(std::vector<math::Vec3>& list, const std::array<uint32_t, 2>& range, geometry::Mesh& mesh, const math::Transform& cameraTransform, const math::Transform& transform) {
    for(uint32_t i = range[0]; i < range[1]; i++) {
        math::Vec3 newVec3 = transform.applyPosition(mesh.getVertex(i).position);
        newVec3 = cameraTransform.applyInverseRotation(newVec3 - cameraTransform.getPosition());
        
        list.at(i) = newVec3;
    }
}

void Renderer::wireframe(std::vector<display::Viewport>& viewports, const Camera& camera, const geometry::World& world)
{
    const float nearPlane = camera.getNearPlane();
    const math::Transform& cameraTransform = camera.getTransform();

    for (const geometry::Model& model : world.getVisibleModels())
    {
        geometry::Mesh& mesh = *model.mesh;
        const math::Transform& modelTransform = model.transform;
        const uint32_t vertexCount = mesh.getNumVertices();
        // const uint32_t faceCount = mesh.getNumFaces();

        std::vector<math::Vec3> viewVertices;
        
        viewVertices.resize(vertexCount);
        uint8_t numThreads = pool->getNumThreads();
        if (numThreads == 0 || vertexCount < 256) {
            transformBatchPosition(viewVertices, {0, vertexCount}, mesh, cameraTransform, modelTransform);
        } else {
            const uint32_t workerCount = std::min<uint32_t>(numThreads, vertexCount);
            const uint32_t chunkSize = (vertexCount + workerCount - 1) / workerCount;

            for (uint32_t thread = 0; thread < workerCount; ++thread) {
                const uint32_t start = thread * chunkSize;
                const uint32_t end = std::min(start + chunkSize, vertexCount);

                pool->addTask([&, start, end]() {
                    transformBatchPosition(
                        viewVertices,
                        {start, end},
                        mesh, cameraTransform, modelTransform);
                });
            }
        }
        pool->waitForCompletion();

        
        for (display::Viewport& viewport : viewports)
        {
            const float areaWidth = static_cast<float>(viewport.areaBounds.right - viewport.areaBounds.left);
            const float areaHeight = static_cast<float>(viewport.areaBounds.bottom - viewport.areaBounds.top);
            const float tileLeftNdc = 2.0f * viewport.tileBounds.left / areaWidth - 1.0f;
            const float tileRightNdc = 2.0f * viewport.tileBounds.right / areaWidth - 1.0f;
            const float tileTopNdc = 1.0f - 2.0f * viewport.tileBounds.top / areaHeight;
            const float tileBottomNdc = 1.0f - 2.0f * viewport.tileBounds.bottom / areaHeight;

            for (uint32_t i = 0; i < mesh.getNumFaces(); ++i)
            {
                //const auto indices = mesh.getTriIndices(i);

                const uint32_t i0 = mesh.getIndice(i * 3 + 0);
                const uint32_t i1 = mesh.getIndice(i * 3 + 1);
                const uint32_t i2 = mesh.getIndice(i * 3 + 2);

                const math::Vec3& a = viewVertices[i0];
                const math::Vec3& b = viewVertices[i1];
                const math::Vec3& c = viewVertices[i2];

                const float depthA = -a.z;
                const float depthB = -b.z;
                const float depthC = -c.z;
                const bool allInFront = depthA >= nearPlane && depthB >= nearPlane && depthC >= nearPlane;

                if (!allInFront && depthA < nearPlane && depthB < nearPlane && depthC < nearPlane)
                    continue;

                if (allInFront &&
                    ((a.x < tileLeftNdc * depthA && b.x < tileLeftNdc * depthB && c.x < tileLeftNdc * depthC) ||
                     (a.x > tileRightNdc * depthA && b.x > tileRightNdc * depthB && c.x > tileRightNdc * depthC) ||
                     (a.y > tileTopNdc * depthA && b.y > tileTopNdc * depthB && c.y > tileTopNdc * depthC) ||
                     (a.y < tileBottomNdc * depthA && b.y < tileBottomNdc * depthB && c.y < tileBottomNdc * depthC)))
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