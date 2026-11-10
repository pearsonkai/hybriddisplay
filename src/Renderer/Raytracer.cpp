#include "Renderer.hpp"
#include <algorithm>
#include <cmath>
#include <limits>

namespace hybriddisplay::rendering {

struct Ray {
    math::Vec3 origin;
    math::Vec3 direction;

    bool intersect(const geometry::Triangle& triangle, float& distance, float& u, float& v) const;
};

bool Ray::intersect(const geometry::Triangle& triangle, float& distance, float& u, float& v) const {
    const math::Vec3 edge1 = triangle.v1->position - triangle.v0->position;
    const math::Vec3 edge2 = triangle.v2->position - triangle.v0->position;
    const math::Vec3 p = direction.cross(edge2);
    const float determinant = edge1.dot(p);
    if (determinant <= 1e-8f)
        return false;

    const math::Vec3 fromVertex = origin - triangle.v0->position;
    u = fromVertex.dot(p) / determinant;
    if (u < 0.0f || u > 1.0f)
        return false;

    const math::Vec3 q = fromVertex.cross(edge1);
    v = direction.dot(q) / determinant;
    if (v < 0.0f || u + v > 1.0f)
        return false;

    distance = edge2.dot(q) / determinant;
    return distance > 0.0f;
}








void Renderer::renderViewport(display::Viewport& viewport, const Camera& camera, const std::vector<geometry::Triangle>& triangles) {
    const float focalLength = camera.getFocalLength();
    if (!(focalLength > 0.0f))
        return;

    const graphics::Resolution resolution = viewport.resolution();
    const uint32_t left = std::min(viewport.tileBounds.left, resolution.width);
    const uint32_t top = std::min(viewport.tileBounds.top, resolution.height);
    const uint32_t right = std::min(viewport.tileBounds.right, resolution.width);
    const uint32_t bottom = std::min(viewport.tileBounds.bottom, resolution.height);
    const float areaWidth = static_cast<float>(viewport.areaBounds.right) - viewport.areaBounds.left;
    const float areaHeight = static_cast<float>(viewport.areaBounds.bottom) - viewport.areaBounds.top;
    if (left >= right || top >= bottom || areaWidth == 0.0f || areaHeight == 0.0f)
        return;

    const float aspect = camera.getAspect(); //areaWidth / areaHeight;
    const math::Vec3 lightDirection = math::Vec3(0.3f, 0.8f, 0.5f).normalize();
    auto& zbuffer = *viewport.zbuffer();
    for (uint32_t y = top; y < bottom; ++y) {
        for (uint32_t x = left; x < right; ++x) {
            const float screenX = 2.0f * (x - viewport.areaBounds.left + 0.5f) / areaWidth - 1.0f;
            const float screenY = 1.0f - 2.0f * (y - viewport.areaBounds.top + 0.5f) / areaHeight;
            Ray ray{math::Vec3(), math::Vec3(screenX * aspect / focalLength, screenY / focalLength, -1.0f).normalize()};
            float depth = std::numeric_limits<float>::max();
            float throughput = 1.0f;
            math::Vec3 colour;

            for (int bounce = 0; bounce < 3 && throughput > 0.01f; ++bounce) {
                float nearestDistance = std::numeric_limits<float>::max();
                const geometry::Triangle* hit = nullptr;
                math::Vec3 normal;
                float hitU = 0.0f, hitV = 0.0f;

                for (const geometry::Triangle& triangle : triangles) {
                    float distance;
                    float u, v;
                    if (ray.intersect(triangle, distance, u, v) && distance < nearestDistance) {
                        nearestDistance = distance;
                        hit = &triangle;
                        hitU = u;
                        hitV = v;
                        normal = (triangle.v0->normal * (1.0f - u - v) + triangle.v1->normal * u + triangle.v2->normal * v).normalize();
                        if (normal.magnitudeSquared() == 0.0f) {
                            normal = (triangle.v1->position - triangle.v0->position).cross(triangle.v2->position - triangle.v0->position).normalize();
                        }
                    }
                }

                if (!hit)
                    break;

                const math::Vec3 position = ray.origin + ray.direction * nearestDistance;
                const float hitDepth = -position.z;
                if (hitDepth < camera.getNearPlane() || hitDepth > camera.getFarPlane())
                    break;
                if (bounce == 0)
                    depth = hitDepth;

                const math::Vec3 texcoord = hit->v0->uv * (1.0f - hitU - hitV) +
                    hit->v1->uv * hitU + hit->v2->uv * hitV;
                const graphics::Colour surface = hit->material->sampleTexture(texcoord.x, texcoord.y);
                const float lighting = 0.2f + 0.8f * std::max(0.0f, normal.dot(lightDirection));
                const float reflectiveness = std::clamp(hit->material->getReflectiveness(), 0.0f, 1.0f);
                colour += math::Vec3(surface.r, surface.g, surface.b) *
                    (throughput * (1.0f - reflectiveness) * lighting);
                throughput *= reflectiveness;

                ray = {
                    position + normal * 0.001f,
                    (ray.direction - normal * (2.0f * ray.direction.dot(normal))).normalize()
                };
            }

            const uint32_t index = y * resolution.width + x;
            if (depth < zbuffer[index]) {
                zbuffer[index] = depth;
                putPixel(viewport, index, graphics::Colour(
                    static_cast<uint8_t>(std::clamp(colour.x, 0.0f, 255.0f)),
                    static_cast<uint8_t>(std::clamp(colour.y, 0.0f, 255.0f)),
                    static_cast<uint8_t>(std::clamp(colour.z, 0.0f, 255.0f))));
            }
        }
    }
}

void Renderer::raytrace(std::vector<display::Viewport>& viewports, const Camera& camera, const geometry::World& world) {
    const float focalLength = camera.getFocalLength();
    if (!(focalLength > 0.0f))
        return;

    size_t vertexCount = 0;
    for (const geometry::Model& model : world.getVisibleModels()) {
        if (model.mesh)
            vertexCount += model.mesh->getNumVertices();
    }

    std::vector<geometry::Vertex> vertices;
    vertices.reserve(vertexCount);
    std::vector<geometry::Triangle> triangles;
    for (const geometry::Model& model : world.getVisibleModels()) {
        if (!model.mesh)
            continue;

        const geometry::Mesh& mesh = *model.mesh;
        const size_t offset = vertices.size();
        const uint32_t modelVertexCount = mesh.getNumVertices();
        std::vector<geometry::Vertex> viewVertices(modelVertexCount);
#ifdef __EMSCRIPTEN__
        transformBatchVertex(viewVertices, {0, modelVertexCount}, mesh, camera.getTransform(), model.transform);
#else
        const uint8_t numThreads = pool ? pool->getNumThreads() : 0;
        if (numThreads == 0 || modelVertexCount < 256) {
            transformBatchVertex(viewVertices, {0, modelVertexCount}, mesh, camera.getTransform(), model.transform);
        } else {
            const uint32_t workerCount = std::min<uint32_t>(numThreads, modelVertexCount);
            const uint32_t chunkSize = (modelVertexCount + workerCount - 1) / workerCount;

            for (uint32_t thread = 0; thread < workerCount; ++thread) {
                const uint32_t start = thread * chunkSize;
                const uint32_t end = std::min(start + chunkSize, modelVertexCount);

                pool->add([&, start, end]() {
                    transformBatchVertex(
                        viewVertices,
                        {start, end},
                        mesh, camera.getTransform(), model.transform);
                });
            }
            pool->wait();
        }
#endif
        vertices.insert(vertices.end(), viewVertices.begin(), viewVertices.end());

        for (uint32_t i = 0; i < mesh.getNumFaces(); ++i) {
            const geometry::Triangle source = mesh.getTri(i);
            if (!source.material)
                continue;

            triangles.push_back({
                &vertices[offset + mesh.getIndice(i * 3)],
                &vertices[offset + mesh.getIndice(i * 3 + 1)],
                &vertices[offset + mesh.getIndice(i * 3 + 2)],
                source.material
            });
        }
    }

    for (display::Viewport& viewport : viewports) {
#ifdef __EMSCRIPTEN__
        renderViewport(viewport, camera, triangles);
#else
        if (pool) {
            pool->add([this, &camera, &triangles, viewportPtr = &viewport]() {
                this->renderViewport(*viewportPtr, camera, triangles);
            });
        } else {
            renderViewport(viewport, camera, triangles);
        }
#endif
    }
#ifndef __EMSCRIPTEN__
    if (pool)
        pool->wait();
#endif
}

}
