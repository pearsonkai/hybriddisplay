#include "Renderer.hpp"
#include <algorithm>
#include <cmath>
#include <iostream>

namespace hybriddisplay::rendering {

void Renderer::skybox(std::vector<display::Viewport>& viewports, const Camera& camera, const graphics::Image<graphics::Colour>& image) {
    if (image.size.width < 4 || image.size.height < 3 ||
        image.data.size() < static_cast<size_t>(image.size.width) * image.size.height) {
        std::cerr << "Invalid skybox image: expected a complete 4x3 cross" << std::endl;
        return;
    }
    const float focalLength = camera.getFocalLength();
    if (!std::isfinite(focalLength) || focalLength <= 0.0f) {
        std::cerr << "Invalid skybox focal length: " << focalLength << std::endl;
        return;
    }

    const math::Vec3& rotation = camera.getTransform().getRotation();
    float sinX = std::sin(rotation.x), cosX = std::cos(rotation.x);
    float sinY = std::sin(rotation.y), cosY = std::cos(rotation.y);
    float sinZ = std::sin(rotation.z), cosZ = std::cos(rotation.z);
    float maxUv = std::nextafter(1.0f, 0.0f);

    const auto drawViewport = [&](display::Viewport& viewport) {
        if (!viewport.screen)
            return;

        const graphics::Resolution resolution = viewport.resolution();
        const uint32_t areaWidth = viewport.areaBounds.right - viewport.areaBounds.left;
        const uint32_t areaHeight = viewport.areaBounds.bottom - viewport.areaBounds.top;
        std::vector<uint32_t>* framebuffer = viewport.framebuffer();
        if (resolution.width == 0 || resolution.height == 0 ||
            areaWidth == 0 || areaHeight == 0 ||
            !framebuffer ||
            framebuffer->size() < static_cast<size_t>(resolution.width) * resolution.height) {
            return;
        }

        const uint32_t left = std::min(viewport.tileBounds.left, resolution.width);
        const uint32_t top = std::min(viewport.tileBounds.top, resolution.height);
        const uint32_t right = std::min(viewport.tileBounds.right, resolution.width);
        const uint32_t bottom = std::min(viewport.tileBounds.bottom, resolution.height);
        const float aspect = static_cast<float>(areaWidth) / areaHeight;

        for (uint32_t y = top; y < bottom; ++y) {
            const float localY = static_cast<float>(y - viewport.areaBounds.top);
            const float normalizedY = (localY + 0.5f) / areaHeight;
            if (normalizedY < viewport.tile.y ||
                normalizedY >= viewport.tile.y + viewport.tile.height) {
                continue;
            }

            for (uint32_t x = left; x < right; ++x) {
                const float localX = static_cast<float>(x - viewport.areaBounds.left);
                const float normalizedX = (localX + 0.5f) / areaWidth;
                if (normalizedX < viewport.tile.x ||
                    normalizedX >= viewport.tile.x + viewport.tile.width) {
                    continue;
                }

                math::Vec3 ray(
                    (2.0f * (localX + 0.5f) / areaWidth - 1.0f) * aspect / focalLength,
                    (1.0f - 2.0f * (localY + 0.5f) / areaHeight) / focalLength,
                    -1.0f);
                ray = math::Vec3(ray.x, ray.y * cosX - ray.z * sinX, ray.y * sinX + ray.z * cosX);
                ray = math::Vec3(ray.x * cosY + ray.z * sinY, ray.y, -ray.x * sinY + ray.z * cosY);
                ray = math::Vec3(ray.x * cosZ - ray.y * sinZ, ray.x * sinZ + ray.y * cosZ, ray.z);

                float ax = std::abs(ray.x), ay = std::abs(ray.y), az = std::abs(ray.z);
                float u, v, major;
                uint32_t faceX, faceY;
                if (ax >= ay && ax >= az) {
                    major = ax;
                    if (ray.x >= 0.0f) {
                        faceX = 2; faceY = 1; u = -ray.z / major; v = -ray.y / major;
                    } else {
                        faceX = 0; faceY = 1; u = ray.z / major; v = -ray.y / major;
                    }
                } else if (ay >= az) {
                    major = ay;
                    if (ray.y >= 0.0f) {
                        faceX = 1; faceY = 0; u = ray.x / major; v = ray.z / major;
                    } else {
                        faceX = 1; faceY = 2; u = ray.x / major; v = -ray.z / major;
                    }
                } else {
                    major = az;
                    if (ray.z >= 0.0f) {
                        faceX = 1; faceY = 1; u = ray.x / major; v = -ray.y / major;
                    } else {
                        faceX = 3; faceY = 1; u = -ray.x / major; v = -ray.y / major;
                    }
                }

                float faceU = std::clamp((u + 1.0f) * 0.5f, 0.0f, maxUv);
                float faceV = std::clamp((v + 1.0f) * 0.5f, 0.0f, maxUv);
                uint32_t faceLeft = static_cast<uint32_t>(static_cast<uint64_t>(faceX) * image.size.width / 4);
                uint32_t faceTop = static_cast<uint32_t>(static_cast<uint64_t>(faceY) * image.size.height / 3);
                uint32_t faceRight = static_cast<uint32_t>(static_cast<uint64_t>(faceX + 1) * image.size.width / 4);
                uint32_t faceBottom = static_cast<uint32_t>(static_cast<uint64_t>(faceY + 1) * image.size.height / 3);
                uint32_t sourceX = faceLeft + std::min(static_cast<uint32_t>(faceU * (faceRight - faceLeft)), faceRight - faceLeft - 1);
                uint32_t sourceY = faceTop + std::min(static_cast<uint32_t>(faceV * (faceBottom - faceTop)), faceBottom - faceTop - 1);
                (*framebuffer)[static_cast<size_t>(y) * resolution.width + x] = image.get(sourceX, sourceY);
            }
        }
    };

    if (!pool) {
        for (display::Viewport& viewport : viewports)
            drawViewport(viewport);
        return;
    }

    for (display::Viewport& viewport : viewports) {
        display::Viewport* viewportPtr = &viewport;
        pool->addTask([&drawViewport, viewportPtr]() {
            drawViewport(*viewportPtr);
        });
    }
    pool->waitForCompletion();
}

};