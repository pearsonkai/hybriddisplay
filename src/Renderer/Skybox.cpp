#include "Renderer.hpp"
#include <algorithm>
#include <cmath>
#include <iostream>

namespace hybriddisplay::rendering {

constexpr uint32_t SKYBOX_COLUMNS = 4;
constexpr uint32_t SKYBOX_ROWS = 3;

void sampleSkyboxFace(const math::Vec3& direction, const graphics::Image<graphics::Colour>& image, uint32_t& sourceX, uint32_t& sourceY) {
    const float absoluteX = std::abs(direction.x);
    const float absoluteY = std::abs(direction.y);
    const float absoluteZ = std::abs(direction.z);

    float u = 0.0f;
    float v = 0.0f;
    uint32_t faceX = 0;
    uint32_t faceY = 0;

    if (absoluteX >= absoluteY && absoluteX >= absoluteZ) {
        const float majorAxis = absoluteX;
        if (direction.x >= 0.0f) {
            faceX = 2; faceY = 1; u = -direction.z / majorAxis; v = -direction.y / majorAxis;
        } else {
            faceX = 0; faceY = 1; u = direction.z / majorAxis; v = -direction.y / majorAxis;
        }
    } else if (absoluteY >= absoluteZ) {
        const float majorAxis = absoluteY;
        if (direction.y >= 0.0f) {
            faceX = 1; faceY = 0; u = direction.x / majorAxis; v = direction.z / majorAxis;
        } else {
            faceX = 1; faceY = 2; u = direction.x / majorAxis; v = -direction.z / majorAxis;
        }
    } else {
        const float majorAxis = absoluteZ;
        if (direction.z >= 0.0f) {
            faceX = 1; faceY = 1; u = direction.x / majorAxis; v = -direction.y / majorAxis;
        } else {
            faceX = 3; faceY = 1; u = -direction.x / majorAxis; v = -direction.y / majorAxis;
        }
    }

    const float sampleU = std::clamp((u + 1.0f) * 0.5f, 0.0f, 1.0f);
    const float sampleV = std::clamp((v + 1.0f) * 0.5f, 0.0f, 1.0f);

    const uint32_t faceLeft = static_cast<uint32_t>(static_cast<uint64_t>(faceX) * image.size.width / SKYBOX_COLUMNS);
    const uint32_t faceTop = static_cast<uint32_t>(static_cast<uint64_t>(faceY) * image.size.height / SKYBOX_ROWS);
    const uint32_t faceRight = static_cast<uint32_t>(static_cast<uint64_t>(faceX + 1) * image.size.width / SKYBOX_COLUMNS);
    const uint32_t faceBottom = static_cast<uint32_t>(static_cast<uint64_t>(faceY + 1) * image.size.height / SKYBOX_ROWS);

    const graphics::Resolution faceSize = {
        std::max<uint32_t>(1u, faceRight - faceLeft),
        std::max<uint32_t>(1u, faceBottom - faceTop)
    };

    sourceX = faceLeft + std::min(static_cast<uint32_t>(sampleU * static_cast<float>(faceSize.width)), faceSize.width - 1u);
    sourceY = faceTop + std::min(static_cast<uint32_t>(sampleV * static_cast<float>(faceSize.height)), faceSize.height - 1u);
}

void drawSkyboxViewport(display::Viewport& viewport, const graphics::Image<graphics::Colour>& image, float focalLength, const math::Vec3& rotation) {
    if (!viewport.screen) {
        return;
    }

    const graphics::Resolution resolution = viewport.resolution();
    const uint32_t areaWidth = viewport.areaBounds.right - viewport.areaBounds.left;
    const uint32_t areaHeight = viewport.areaBounds.bottom - viewport.areaBounds.top;
    std::vector<uint32_t>* framebuffer = viewport.framebuffer();
    
    if (resolution.width == 0 || resolution.height == 0 ||
        areaWidth == 0 || areaHeight == 0 ||
        framebuffer == nullptr ||
        framebuffer->size() < static_cast<size_t>(resolution.width) * resolution.height) {
        return;
    }

    const float sinX = std::sin(rotation.x);
    const float cosX = std::cos(rotation.x);
    const float sinY = std::sin(rotation.y);
    const float cosY = std::cos(rotation.y);
    const float sinZ = std::sin(rotation.z);
    const float cosZ = std::cos(rotation.z);

    const uint32_t left = std::min(viewport.tileBounds.left, resolution.width);
    const uint32_t top = std::min(viewport.tileBounds.top, resolution.height);
    const uint32_t right = std::min(viewport.tileBounds.right, resolution.width);
    const uint32_t bottom = std::min(viewport.tileBounds.bottom, resolution.height);
    const float aspect = static_cast<float>(areaWidth) / static_cast<float>(areaHeight);

    for (uint32_t y = top; y < bottom; ++y) {
        const float localY = static_cast<float>(y - viewport.areaBounds.top);
        const float normalizedY = (localY + 0.5f) / static_cast<float>(areaHeight);
        if (normalizedY < viewport.tile.y || normalizedY >= viewport.tile.y + viewport.tile.height) {
            continue;
        }

        for (uint32_t x = left; x < right; ++x) {
            const float localX = static_cast<float>(x - viewport.areaBounds.left);
            const float normalizedX = (localX + 0.5f) / static_cast<float>(areaWidth);
            if (normalizedX < viewport.tile.x || normalizedX >= viewport.tile.x + viewport.tile.width) {
                continue;
            }

            math::Vec3 ray(
                (2.0f * (localX + 0.5f) / static_cast<float>(areaWidth) - 1.0f) * aspect / focalLength,
                (1.0f - 2.0f * (localY + 0.5f) / static_cast<float>(areaHeight)) / focalLength,
                -1.0f);
            ray = math::Vec3(ray.x, ray.y * cosX - ray.z * sinX, ray.y * sinX + ray.z * cosX);
            ray = math::Vec3(ray.x * cosY + ray.z * sinY, ray.y, -ray.x * sinY + ray.z * cosY);
            ray = math::Vec3(ray.x * cosZ - ray.y * sinZ, ray.x * sinZ + ray.y * cosZ, ray.z);

            uint32_t sourceX = 0;
            uint32_t sourceY = 0;
            sampleSkyboxFace(ray, image, sourceX, sourceY);
            (*framebuffer)[static_cast<size_t>(y) * resolution.width + x] = image.get(sourceX, sourceY);
        }
    }
}

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

    const math::Vec3 rotation = camera.getTransform().getRotation();

    if (pool == nullptr) {
        for (display::Viewport& viewport : viewports) {
            drawSkyboxViewport(viewport, image, focalLength, rotation);
        }
        return;
    }

    for (display::Viewport& viewport : viewports) {
        display::Viewport* viewportPtr = &viewport;
        pool->addTask([&image, focalLength, rotation, viewportPtr]() {
            drawSkyboxViewport(*viewportPtr, image, focalLength, rotation);
        });
    }
    pool->waitForCompletion();
}

};