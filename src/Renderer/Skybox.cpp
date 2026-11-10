#include "Renderer.hpp"
#include <algorithm>
#include <cmath>
#include <iostream>

namespace hybriddisplay::rendering {

constexpr uint32_t SKYBOX_COLUMNS = 4;
constexpr uint32_t SKYBOX_ROWS = 3;

graphics::Colour sampleSkyboxFace(const math::Vec3& direction, const graphics::Image<graphics::Colour>& image) {
    const float absX = std::abs(direction.x), absY = std::abs(direction.y), absZ = std::abs(direction.z);
    float u, v;
    uint32_t faceX, faceY;

    if (absX >= absY && absX >= absZ) {
        faceX = direction.x >= 0.0f ? 2 : 0;
        faceY = 1;
        u = (direction.x >= 0.0f ? -direction.z : direction.z) / absX;
        v = -direction.y / absX;
    } else if (absY >= absZ) {
        faceX = 1;
        faceY = direction.y >= 0.0f ? 0 : 2;
        u = direction.x / absY;
        v = (direction.y >= 0.0f ? direction.z : -direction.z) / absY;
    } else {
        faceX = direction.z >= 0.0f ? 1 : 3;
        faceY = 1;
        u = (direction.z >= 0.0f ? direction.x : -direction.x) / absZ;
        v = -direction.y / absZ;
    }

    const uint32_t faceLeft = static_cast<uint64_t>(faceX) * image.size.width / SKYBOX_COLUMNS;
    const uint32_t faceTop = static_cast<uint64_t>(faceY) * image.size.height / SKYBOX_ROWS;
    const uint32_t faceWidth = std::max(1u, static_cast<uint32_t>(static_cast<uint64_t>(faceX + 1) * image.size.width / SKYBOX_COLUMNS) - faceLeft);
    const uint32_t faceHeight = std::max(1u, static_cast<uint32_t>(static_cast<uint64_t>(faceY + 1) * image.size.height / SKYBOX_ROWS) - faceTop);
    const float pixelX = std::clamp((u + 1.0f) * 0.5f, 0.0f, 1.0f) * faceWidth - 0.5f;
    const float pixelY = std::clamp((v + 1.0f) * 0.5f, 0.0f, 1.0f) * faceHeight - 0.5f;
    const int32_t x = static_cast<int32_t>(std::floor(pixelX)), y = static_cast<int32_t>(std::floor(pixelY));
    const float blendX = pixelX - x, blendY = pixelY - y;
    const graphics::Colour& topLeft = image.get(faceLeft + std::clamp(x, 0, static_cast<int32_t>(faceWidth) - 1), faceTop + std::clamp(y, 0, static_cast<int32_t>(faceHeight) - 1));
    const graphics::Colour& topRight = image.get(faceLeft + std::clamp(x + 1, 0, static_cast<int32_t>(faceWidth) - 1), faceTop + std::clamp(y, 0, static_cast<int32_t>(faceHeight) - 1));
    const graphics::Colour& bottomLeft = image.get(faceLeft + std::clamp(x, 0, static_cast<int32_t>(faceWidth) - 1), faceTop + std::clamp(y + 1, 0, static_cast<int32_t>(faceHeight) - 1));
    const graphics::Colour& bottomRight = image.get(faceLeft + std::clamp(x + 1, 0, static_cast<int32_t>(faceWidth) - 1), faceTop + std::clamp(y + 1, 0, static_cast<int32_t>(faceHeight) - 1));
    
    return graphics::Colour::blend(topLeft, topRight, bottomLeft, bottomRight, blendX, blendY);
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

    const float aspect = static_cast<float>(areaWidth) / static_cast<float>(areaHeight);

    for (uint32_t y = std::min(viewport.tileBounds.top, resolution.height); y < std::min(viewport.tileBounds.bottom, resolution.height); ++y) {
        const float localY = y - viewport.areaBounds.top;
        const float normalizedY = (localY + 0.5f) / areaHeight;
        if (normalizedY < viewport.tile.y || normalizedY >= viewport.tile.y + viewport.tile.height) continue;

        for (uint32_t x = std::min(viewport.tileBounds.left, resolution.width); x < std::min(viewport.tileBounds.right, resolution.width); ++x) {
            const float localX = x - viewport.areaBounds.left;
            const float normalizedX = (localX + 0.5f) / areaWidth;
            if (normalizedX < viewport.tile.x || normalizedX >= viewport.tile.x + viewport.tile.width) continue;

            math::Vec3 ray(
                (2.0f * (localX + 0.5f) / static_cast<float>(areaWidth) - 1.0f) * aspect / focalLength,
                (1.0f - 2.0f * (localY + 0.5f) / static_cast<float>(areaHeight)) / focalLength,
                -1.0f);
            ray = math::Vec3(ray.x, ray.y * cosX - ray.z * sinX, ray.y * sinX + ray.z * cosX);
            ray = math::Vec3(ray.x * cosY + ray.z * sinY, ray.y, -ray.x * sinY + ray.z * cosY);
            ray = math::Vec3(ray.x * cosZ - ray.y * sinZ, ray.x * sinZ + ray.y * cosZ, ray.z);

            (*framebuffer)[static_cast<size_t>(y) * resolution.width + x] = sampleSkyboxFace(ray, image);
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
#ifdef __EMSCRIPTEN__
    for (display::Viewport& viewport : viewports)
        drawSkyboxViewport(viewport, image, focalLength, rotation);
#else
    if (pool == nullptr) {
        for (display::Viewport& viewport : viewports)
            drawSkyboxViewport(viewport, image, focalLength, rotation);
        return;
    }

    for (display::Viewport& viewport : viewports) {
        pool->add([&image, focalLength, rotation, viewportPtr = &viewport]() {
            drawSkyboxViewport(*viewportPtr, image, focalLength, rotation);
        });
    }
    pool->wait();
#endif
}

};