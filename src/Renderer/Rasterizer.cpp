#include "Renderer.hpp"
#include <algorithm>
#include <array>
#include <cmath>

namespace hybriddisplay::rendering {


void Renderer::drawTriangle(display::Viewport& viewport, const geometry::Triangle& tri, const Camera& camera, const math::Transform& modelTransform)
{
    const graphics::Material* mat = tri.material;
    if (viewport.tileBounds.left >= viewport.tileBounds.right ||
        viewport.tileBounds.top >= viewport.tileBounds.bottom)
        return;
    
    math::Vec3 inputPosition[] = {tri.v0->position, tri.v1->position, tri.v2->position};
    math::Vec3 inputUV[] = {tri.v0->uv, tri.v1->uv, tri.v2->uv};
    
    /*
    math::Vec3 toCamera = ((inputPosition[0] + inputPosition[1] + inputPosition[2]) / -3.0f).normalize();
    float lighting = std::max(0.0f, normal.dot(toCamera));
    */
    math::Vec3 face_normal = (inputPosition[1] - inputPosition[0]).cross(inputPosition[2] - inputPosition[0]).normalize();
    math::Vec3 toCamera = ((inputPosition[0] + inputPosition[1] + inputPosition[2]) / -3.0f).normalize();
    //float face_lighting = std::max(0.0f, face_normal.dot(toCamera));
    
    math::Vec3 edge1 = inputPosition[1] - inputPosition[0], edge2 = inputPosition[2] - inputPosition[0];
    math::Vec3 uv1 = inputUV[1] - inputUV[0], uv2 = inputUV[2] - inputUV[0];
    float uvDet = uv1.x * uv2.y - uv1.y * uv2.x;
    math::Vec3 tangent = edge1, bitangent = face_normal.cross(edge1);
    if (std::abs(uvDet) > 1e-8f) {
        tangent = (edge1 * uv2.y - edge2 * uv1.y) / uvDet;
        bitangent = (edge2 * uv1.x - edge1 * uv2.x) / uvDet;
    }
    tangent = tangent.normalize();
    bitangent = bitangent.normalize();
    

    struct ClipVertex {
        math::Vec3 position;
        math::Vec3 uv;
    };
    std::array<ClipVertex, 8> polygon;
    for (size_t i = 0; i < 3; ++i) {
        polygon[i] = {inputPosition[i], inputUV[i]};
    }
    size_t count = 3;
    const float nearPlane = camera.getNearPlane();
    const float inverseFocalLength = 1.0f / camera.getFocalLength();

    const auto clipAgainst = [&](auto signedDistance) {
        if (count == 0) return;

        std::array<ClipVertex, 8> clipped{};
        size_t clippedCount = 0;

        size_t previous = count - 1;
        float previousDistance = signedDistance(polygon[previous].position);
        bool previousInside = previousDistance >= 0.0f;

        for (size_t current = 0; current < count; ++current) {
            const float currentDistance = signedDistance(polygon[current].position);
            const bool currentInside = currentDistance >= 0.0f;

            if (previousInside != currentInside) {
                const float t = previousDistance / (previousDistance - currentDistance);
                if (t > 0.0f && t < 1.0f) {
                    clipped[clippedCount++] = {
                        polygon[previous].position + (polygon[current].position - polygon[previous].position) * t,
                        polygon[previous].uv + (polygon[current].uv - polygon[previous].uv) * t
                    };
                }
            }
            if (currentInside) {
                clipped[clippedCount++] = polygon[current];
            }

            previous = current;
            previousDistance = currentDistance;
            previousInside = currentInside;
        }

        polygon = clipped;
        count = clippedCount;
    };

    clipAgainst([&](const math::Vec3& point) { return -point.z - nearPlane; });
    clipAgainst([&](const math::Vec3& point) { return point.x - point.z * inverseFocalLength; });
    clipAgainst([&](const math::Vec3& point) { return -point.x - point.z * inverseFocalLength; });
    clipAgainst([&](const math::Vec3& point) { return point.y - point.z * inverseFocalLength; });
    clipAgainst([&](const math::Vec3& point) { return -point.y - point.z * inverseFocalLength; });

    if (count < 3) return;

    float width = static_cast<float>(viewport.areaBounds.right - viewport.areaBounds.left);
    float height = static_cast<float>(viewport.areaBounds.bottom - viewport.areaBounds.top);
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
    if (bounds.left >= bounds.right || bounds.top >= bounds.bottom) return;

    for (size_t t = 1; t + 1 < count; ++t) {
        size_t a = 0, b = t, c = t + 1;
        float denominator = (screen[b].y - screen[c].y) * (screen[a].x - screen[c].x) + (screen[c].x - screen[b].x) * (screen[a].y - screen[c].y);
        if (denominator == 0.0f) continue;

        for (uint32_t i = bounds.top; i < bounds.bottom; i++) {
            for (uint32_t j = bounds.left; j < bounds.right; j++) {
                float u = ((screen[b].y - screen[c].y) * (j - screen[c].x) + (screen[c].x - screen[b].x) * (i - screen[c].y)) / denominator;
                float v = ((screen[c].y - screen[a].y) * (j - screen[c].x) + (screen[a].x - screen[c].x) * (i - screen[c].y)) / denominator;
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
                        
                        /*
                        math::Vec3 normal = camera.getTransform().applyInverseRotation(modelTransform.applyRotation(normal)).normalize();
                        lighting = std::max(0.0f, normal.dot(toCamera));
                        normal = (tangent * normal.x + bitangent * normal.y + face_normal * normal.z).normalize();
                        */
                        
                        float x = tri.v0->normal.x * u + tri.v1->normal.x * v + tri.v2->normal.x * w;
                        float y = tri.v0->normal.y * u + tri.v1->normal.y * v + tri.v2->normal.y * w;
                        float z = tri.v0->normal.z * u + tri.v1->normal.z * v + tri.v2->normal.z * w; 
                        math::Vec3 pixel_normal = math::Vec3(x,y,z);
                        float lighting = std::max(0.0f, pixel_normal.dot(toCamera));
                        lighting = 1;
                        colour = graphics::Colour(
                            static_cast<uint8_t>(colour.r * lighting),
                            static_cast<uint8_t>(colour.g * lighting),
                            static_cast<uint8_t>(colour.b * lighting),
                            colour.a);
                        Renderer::putPixel(viewport, j, i, colour);
                    }
                }
            }
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
            
            drawTriangle(viewport, tri, camera, modelTransform);
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