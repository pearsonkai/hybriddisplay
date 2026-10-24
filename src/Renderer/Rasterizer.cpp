#include "Renderer.hpp"
#include <algorithm>
#include <array>
#include <cmath>

namespace hybriddisplay::rendering {

void clipPolygonAgainstPlane(std::array<geometry::Vertex, 8>& polygon, size_t& count, const math::Vec3& planeNormal, float planeOffset)
{
    if (count == 0) {
        return;
    }

    std::array<geometry::Vertex, 8> clippedPolygon{};
    size_t clippedCount = 0;
    size_t previousIndex = count - 1;
    float previousValue = polygon[previousIndex].position.dot(planeNormal) + planeOffset;
    bool previousInside = previousValue >= 0.0f;

    for (size_t currentIndex = 0; currentIndex < count; ++currentIndex) {
        const float currentValue = polygon[currentIndex].position.dot(planeNormal) + planeOffset;
        const bool currentInside = currentValue >= 0.0f;

        if (previousInside != currentInside) {
            const float t = previousValue / (previousValue - currentValue);
            if (t > 0.0f && t < 1.0f) {
                const geometry::Vertex& start = polygon[previousIndex];
                const geometry::Vertex& end = polygon[currentIndex];
                clippedPolygon[clippedCount++] = {
                    start.position + (end.position - start.position) * t,
                    start.normal + (end.normal - start.normal) * t,
                    start.uv + (end.uv - start.uv) * t
                };
            }
        }
        if (currentInside)
            clippedPolygon[clippedCount++] = polygon[currentIndex];

        previousIndex = currentIndex;
        previousValue = currentValue;
        previousInside = currentInside;
    }

    polygon = clippedPolygon;
    count = clippedCount;
}

void Renderer::drawTriangle(display::Viewport& viewport, const geometry::Triangle& tri, const Camera& camera)
{
    const graphics::Material* mat = tri.material;
    if (viewport.tileBounds.left >= viewport.tileBounds.right || viewport.tileBounds.top >= viewport.tileBounds.bottom) {
        return;
    }
    
    math::Vec3 toCamera = ((tri.v0->position + tri.v1->position + tri.v2->position) / -3.0f).normalize();
    
    std::array<geometry::Vertex, 8> polygon{};
    polygon[0] = *tri.v0;
    polygon[1] = *tri.v1;
    polygon[2] = *tri.v2;
    size_t count = 3;
    const float nearPlane = camera.getNearPlane();
    const float inverseFocalLength = 1.0f / camera.getFocalLength();
    const float width = static_cast<float>(viewport.areaBounds.right - viewport.areaBounds.left);
    const float height = static_cast<float>(viewport.areaBounds.bottom - viewport.areaBounds.top);
    const float horizontalClip = (width / height) * inverseFocalLength;

    clipPolygonAgainstPlane(polygon, count, math::Vec3(0.0f, 0.0f, -1.0f), -nearPlane);
    clipPolygonAgainstPlane(polygon, count, math::Vec3(1.0f, 0.0f, -horizontalClip), 0.0f);
    clipPolygonAgainstPlane(polygon, count, math::Vec3(-1.0f, 0.0f, -horizontalClip), 0.0f);
    clipPolygonAgainstPlane(polygon, count, math::Vec3(0.0f, 1.0f, -inverseFocalLength), 0.0f);
    clipPolygonAgainstPlane(polygon, count, math::Vec3(0.0f, -1.0f, -inverseFocalLength), 0.0f);

    if (count < 3) return;

    std::array<math::Vec3, 8> screen;
    for (size_t i = 0; i < count; ++i) {
        screen[i] = camera.projectView(polygon[i].position, width, height);
        screen[i].x += viewport.areaBounds.left;
        screen[i].y += viewport.areaBounds.top;
    }

    float minX = screen[0].x, maxX = minX;
    float minY = screen[0].y, maxY = minY;
    for (size_t i = 1; i < count; ++i) {
        minX = std::min(minX, screen[i].x); maxX = std::max(maxX, screen[i].x);
        minY = std::min(minY, screen[i].y); maxY = std::max(maxY, screen[i].y);
    }

    display::Bounds bounds{
        static_cast<uint32_t>(std::clamp(std::floor(minX), static_cast<float>(viewport.tileBounds.left), static_cast<float>(viewport.tileBounds.right))),
        static_cast<uint32_t>(std::clamp(std::floor(minY), static_cast<float>(viewport.tileBounds.top), static_cast<float>(viewport.tileBounds.bottom))),
        static_cast<uint32_t>(std::clamp(std::ceil(maxX), static_cast<float>(viewport.tileBounds.left), static_cast<float>(viewport.tileBounds.right))),
        static_cast<uint32_t>(std::clamp(std::ceil(maxY), static_cast<float>(viewport.tileBounds.top), static_cast<float>(viewport.tileBounds.bottom)))
    };
    
    if (bounds.left >= bounds.right || bounds.top >= bounds.bottom) {
        return;
    }

    for (uint32_t t = 1; t + 1 < count; t++) {
        uint32_t a = 0, b = t, c = t + 1;
        float denominator = (screen[b].y - screen[c].y) * (screen[a].x - screen[c].x) + (screen[c].x - screen[b].x) * (screen[a].y - screen[c].y);
        if (denominator == 0.0f) continue;

        const float uStepX = (screen[b].y - screen[c].y) / denominator;
        const float uStepY = (screen[c].x - screen[b].x) / denominator;
        const float vStepX = (screen[c].y - screen[a].y) / denominator;
        const float vStepY = (screen[a].x - screen[c].x) / denominator;
        const float uRowStart = ((screen[b].y - screen[c].y) * (bounds.left - screen[c].x) + (screen[c].x - screen[b].x) * (bounds.top - screen[c].y)) / denominator;
        const float vRowStart = ((screen[c].y - screen[a].y) * (bounds.left - screen[c].x) + (screen[a].x - screen[c].x) * (bounds.top - screen[c].y)) / denominator;

        float uStart = uRowStart;
        float vStart = vRowStart;
        for (uint32_t i = bounds.top; i < bounds.bottom; i++) {
            float u = uStart;
            float v = vStart;
            for (uint32_t j = bounds.left; j < bounds.right; j++, u += uStepX, v += vStepX) {
                float w = 1.0f - u - v;

                if (u >= 0.0f && v >= 0.0f && w >= 0.0f) {
                    
                    const float reciprocalDepth =
                        u / -polygon[a].position.z +
                        v / -polygon[b].position.z +
                        w / -polygon[c].position.z;
                    const float depth = 1.0f / reciprocalDepth;
                    uint32_t index = i * viewport.resolution().width + j;

                    if (depth < viewport.zbuffer()->at(index)) {
                        
                        const math::Vec3 texcoord = (
                            polygon[a].uv * (u / -polygon[a].position.z) +
                            polygon[b].uv * (v / -polygon[b].position.z) +
                            polygon[c].uv * (w / -polygon[c].position.z)
                        ) / reciprocalDepth;
                        graphics::Colour colour = mat->sampleTexture(texcoord.x, texcoord.y);
                        if(colour.a == 0) continue; // skip transparent pixels
                        
                        viewport.zbuffer()->at(index) = depth;
                        
                        const math::Vec3 pixelNormal = (
                            polygon[a].normal * (u / -polygon[a].position.z) +
                            polygon[b].normal * (v / -polygon[b].position.z) +
                            polygon[c].normal * (w / -polygon[c].position.z)
                        ).normalize();

                        float lighting = std::max(0.3f, pixelNormal.dot(toCamera));

                        
                        //colour = graphics::COLOUR_WHITE;
                        //colour = graphics::Colour(pixelNormal);
                        //lighting = 1;

                        colour = graphics::Colour(
                            static_cast<uint8_t>(colour.r * lighting),
                            static_cast<uint8_t>(colour.g * lighting),
                            static_cast<uint8_t>(colour.b * lighting),
                            colour.a);

                        Renderer::putPixel(viewport, j, i, colour);
                    }
                }
            }
            uStart += uStepY;
            vStart += vStepY;
        }
    }
}


void Renderer::rasterize(std::vector<display::Viewport>& viewports, const Camera& camera, const geometry::World& world)
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
        auto drawViewport = [&](display::Viewport& viewport) {
        for (uint32_t i = 0; i < mesh.getNumFaces(); ++i) {
            const uint32_t i0 = mesh.getIndice(i * 3 + 0);
            const uint32_t i1 = mesh.getIndice(i * 3 + 1);
            const uint32_t i2 = mesh.getIndice(i * 3 + 2);

            const geometry::Vertex& a = viewVertices[i0];
            const geometry::Vertex& b = viewVertices[i1];
            const geometry::Vertex& c = viewVertices[i2];

            const float depthA = -a.position.z;
            const float depthB = -b.position.z;
            const float depthC = -c.position.z;
            const bool allInFront =
                depthA >= nearPlane && depthB >= nearPlane && depthC >= nearPlane;

            if (!allInFront && depthA < nearPlane && depthB < nearPlane && depthC < nearPlane)
                continue;
            
            math::Vec3 normal = (b.position - a.position).cross(c.position - a.position);
            math::Vec3 toCamera = (a.position + b.position + c.position) / -3.0f;
            if (normal.dot(toCamera) <= 0.0f)
                continue;

            graphics::Material* mat = mesh.getMaterial(i);
            geometry::Triangle tri = {&a,&b,&c,mat};
            
            drawTriangle(viewport, tri, camera);
        }};


        for (display::Viewport& viewport : viewports)
        {
            display::Viewport* ptr_viewport = &viewport;
            pool->addTask([&, ptr_viewport]() { drawViewport(*ptr_viewport); });
        }
        
        pool->waitForCompletion();
    }

}


};