#include "Renderer.hpp"
#include <algorithm>
#include <cmath>

namespace hybriddisplay::rendering {

void drawClippedLine(display::Viewport& viewport, const Camera& camera, const math::Vec3& view0, const math::Vec3& view1, const graphics::Colour& colour) {
    
    const float areaWidth = static_cast<float>(viewport.areaBounds.right - viewport.areaBounds.left);
    const float areaHeight = static_cast<float>(viewport.areaBounds.bottom - viewport.areaBounds.top);
    
    float nearPlane = camera.getNearPlane();
    float depth0 = -view0.z;
    float depth1 = -view1.z;

    // Entirely behind near plane
    if (depth0 < nearPlane && depth1 < nearPlane)
        return;

    math::Vec3 a = view0;
    math::Vec3 b = view1;

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

    Renderer::drawLine(viewport, camera.projectView(a, areaWidth, areaHeight), camera.projectView(b, areaWidth, areaHeight), colour);
}


bool Renderer::clipLineToBounds(const display::Viewport& viewport, math::Vec3& p0, math::Vec3& p1)
{
    const float left = static_cast<float>(viewport.tileBounds.left);
    const float top = static_cast<float>(viewport.tileBounds.top);
    const float right = static_cast<float>(viewport.tileBounds.right) - 1.0f;
    const float bottom = static_cast<float>(viewport.tileBounds.bottom) - 1.0f;

    if (right < left || bottom < top)
        return false;

    const float dx = p1.x - p0.x;
    const float dy = p1.y - p0.y;
    float entry = 0.0f;
    float exit = 1.0f;

    const auto clipAxis = [&](float start, float delta, float minimum, float maximum) {
        if (delta == 0.0f)
            return start >= minimum && start <= maximum;

        float axisEntry = (minimum - start) / delta;
        float axisExit = (maximum - start) / delta;
        if (axisEntry > axisExit)
            std::swap(axisEntry, axisExit);

        entry = std::max(entry, axisEntry);
        exit = std::min(exit, axisExit);
        return entry <= exit;
    };

    if (!clipAxis(p0.x, dx, left, right) || !clipAxis(p0.y, dy, top, bottom))
        return false;

    const math::Vec3 start = p0;
    p0 = start + (p1 - start) * entry;
    p1 = start + (p1 - start) * exit;
    return true;
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

                drawClippedLine(viewport, camera, a, b, graphics::COLOUR_MAGENTA);
                drawClippedLine(viewport, camera, b, c, graphics::COLOUR_MAGENTA);
                drawClippedLine(viewport, camera, c, a, graphics::COLOUR_MAGENTA);
            }
        }
    }
}

};