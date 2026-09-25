#include "Renderer.hpp"
#include <algorithm>
#include <cmath>

namespace hybriddisplay::rendering {



    
void drawTriangle(const display::Viewport& viewport, const geometry::Triangle& tri) 
{
    const geometry::Vertex& v0 = (*tri.v0);
    const geometry::Vertex& v1 = (*tri.v1);
    const geometry::Vertex& v2 = (*tri.v2);
    const graphics::Material* mat = tri.material;
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
            math::Vec3 toCamera = camera.getTransform().getPosition() - ( (a.position + b.position + c.position) / 3);
            if (normal.dot(toCamera) <= 0)
                continue;

            graphics::Material* mat = mesh.getMaterial(i);
            geometry::Triangle tri = {&a,&b,&c,mat};
            drawTriangle(viewport,tri);
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