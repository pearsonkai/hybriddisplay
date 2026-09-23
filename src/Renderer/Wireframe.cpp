#include "Renderer.hpp"
#include <algorithm>
#include <cmath>

namespace hybriddisplay::rendering {

void drawClippedLine(display::Viewport& viewport, const Camera& camera, const math::Vec3& view0, const math::Vec3& view1, const graphics::Colour& colour) {
        
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
    graphics::Resolution res = viewport.resolution();

    Renderer::drawLine(viewport, camera.projectView(a, res.width, res.height), camera.projectView(b, res.width, res.height), colour);
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


void Renderer::wireframe(std::vector<display::Viewport>& viewports, const Camera& camera, const geometry::World& world)
{
    const float nearPlane = camera.getNearPlane();
    const math::Transform& cameraTransform = camera.getTransform();

    for (const geometry::Model& model : world.getVisibleModels())
    {
        const geometry::Mesh& mesh = *model.mesh;
        const math::Transform& modelTransform = model.transform;
        const uint32_t vertexCount = mesh.getNumVertices();
        // const uint32_t faceCount = mesh.getNumFaces();

        std::vector<geometry::Vertex> viewVertices;
        
        viewVertices.resize(vertexCount);
        uint8_t numThreads = pool->getNumThreads();
        if (numThreads == 0 || vertexCount < 256) {
            transformBatchVertex(viewVertices, {0, vertexCount}, mesh, cameraTransform, modelTransform);
        } else {
            const uint32_t workerCount = std::min<uint32_t>(numThreads, vertexCount);
            const uint32_t chunkSize = (vertexCount + workerCount - 1) / workerCount;

            for (uint32_t thread = 0; thread < workerCount; ++thread) {
                const uint32_t start = thread * chunkSize;
                const uint32_t end = std::min(start + chunkSize, vertexCount);

                pool->addTask([&, start, end]() {
                    transformBatchVertex(
                        viewVertices,
                        {start, end},
                        mesh, cameraTransform, modelTransform);
                });
            }
        }

        pool->waitForCompletion();
        
        for (display::Viewport& viewport : viewports)
        {
            for (uint32_t i = 0; i < mesh.getNumFaces(); ++i)
            {
                const uint32_t i0 = mesh.getIndice(i * 3 + 0);
                const uint32_t i1 = mesh.getIndice(i * 3 + 1);
                const uint32_t i2 = mesh.getIndice(i * 3 + 2);

                const math::Vec3& a = viewVertices[i0].position;
                const math::Vec3& b = viewVertices[i1].position;
                const math::Vec3& c = viewVertices[i2].position;

                const float depthA = -a.z;
                const float depthB = -b.z;
                const float depthC = -c.z;
                const bool allInFront = depthA >= nearPlane && depthB >= nearPlane && depthC >= nearPlane;

                if (!allInFront && depthA < nearPlane && depthB < nearPlane && depthC < nearPlane)
                    continue;

                if (allInFront && viewport.triangleOutside(camera.getFocalLength(), a, b, c))
                    continue;

                drawClippedLine(viewport, camera, a, b, graphics::COLOUR_MAGENTA);
                drawClippedLine(viewport, camera, b, c, graphics::COLOUR_MAGENTA);
                drawClippedLine(viewport, camera, c, a, graphics::COLOUR_MAGENTA);
            }
        }
    }
}

};