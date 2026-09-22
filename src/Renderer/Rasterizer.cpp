#include "Renderer.hpp"
#include <algorithm>
#include <cmath>

namespace hybriddisplay::rendering {



    
void drawTriangle(const display::Viewport& viewport, const geometry::Triangle& tri) 
{

}

void Renderer::rasterize(std::vector<display::Viewport>& viewports, const Camera& camera, const geometry::World& world)
{
    const float nearPlane = camera.getNearPlane();
    const math::Transform& cameraTransform = camera.getTransform();

    for (const geometry::Model& model : world.getVisibleModels())
    {
        geometry::Mesh& mesh = *model.mesh;
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
            const float areaWidth = static_cast<float>(viewport.areaBounds.right - viewport.areaBounds.left);
            const float areaHeight = static_cast<float>(viewport.areaBounds.bottom - viewport.areaBounds.top);
            
            const float tileLeftNdc = 2.0f * viewport.tileBounds.left / areaWidth - 1.0f;
            const float tileRightNdc = 2.0f * viewport.tileBounds.right / areaWidth - 1.0f;
            const float tileTopNdc = 1.0f - 2.0f * viewport.tileBounds.top / areaHeight;
            const float tileBottomNdc = 1.0f - 2.0f * viewport.tileBounds.bottom / areaHeight;

            for (uint32_t i = 0; i < mesh.getNumFaces(); ++i)
            {
                const uint32_t i0 = mesh.getIndice(i * 3 + 0);
                const uint32_t i1 = mesh.getIndice(i * 3 + 1);
                const uint32_t i2 = mesh.getIndice(i * 3 + 2);

                const geometry::Vertex& a = viewVertices[i0];
                const geometry::Vertex& b = viewVertices[i1];
                const geometry::Vertex& c = viewVertices[i2];

                const float depthA = -a.position.z;
                const float depthB = -b.position.z;
                const float depthC = -c.position.z;

                const bool allInFront = depthA >= nearPlane && depthB >= nearPlane && depthC >= nearPlane;

                if (!allInFront && depthA < nearPlane && depthB < nearPlane && depthC < nearPlane)
                    continue;

                /*
                if (allInFront &&
                    ((a.x < tileLeftNdc * depthA && b.x < tileLeftNdc * depthB && c.x < tileLeftNdc * depthC) ||
                     (a.x > tileRightNdc * depthA && b.x > tileRightNdc * depthB && c.x > tileRightNdc * depthC) ||
                     (a.y > tileTopNdc * depthA && b.y > tileTopNdc * depthB && c.y > tileTopNdc * depthC) ||
                     (a.y < tileBottomNdc * depthA && b.y < tileBottomNdc * depthB && c.y < tileBottomNdc * depthC)))
                {
                    continue;
                }*/
            }
        }
    }
}

};