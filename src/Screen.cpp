#include <cmath>
#include <algorithm>
#include <array>
#include <cstring>
#include <iostream>
#include <limits>
#include "Screen.hpp"

namespace hybriddisplay::display {

Screen::Screen(const graphics::Resolution& windowRes, const graphics::Resolution& renderRes){
    window = SDL_CreateWindow("hybriddisplay", windowRes.width, windowRes.height, SDL_WINDOW_RESIZABLE);
    if (!window) {
        std::cerr << "SDL_CreateWindow failed: " << SDL_GetError() << std::endl;
    }

    renderer = SDL_CreateRenderer(window, NULL);
    if (!renderer) {
               std::cerr << "SDL_CreateRenderer failed: " << SDL_GetError() << std::endl;
    }

    resizeRender(renderRes);
}

Screen::Screen(const graphics::Resolution& windowRes) : Screen(windowRes, windowRes) {
    
}

Screen::~Screen() {
    SDL_DestroyTexture(texture);
    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);
}


graphics::Resolution Screen::getWindowRes() {
    int width;
    int height;

    SDL_GetWindowSizeInPixels(window, &width, &height);
    return {static_cast<uint32_t>(width),static_cast<uint32_t>(height)};
}

graphics::Resolution Screen::getRenderRes() {
    return resolution;
}

void Screen::resizeWindow(const graphics::Resolution& windowRes) {
    SDL_SetWindowSize(window, windowRes.width, windowRes.height);
}

void Screen::resizeRender(const graphics::Resolution& renderRes) {
    if (renderRes.width == 0 || renderRes.height == 0 ||
        renderRes.width > static_cast<uint32_t>(std::numeric_limits<int>::max()) ||
        renderRes.height > static_cast<uint32_t>(std::numeric_limits<int>::max())) {
        std::cerr << "Invalid render resolution: " << renderRes.width << "x" << renderRes.height << std::endl;
        return;
    }

    if (renderRes.width == resolution.width && renderRes.height == resolution.height)
        return;

    const size_t pixelCount = static_cast<size_t>(renderRes.width) * renderRes.height;
    std::vector<uint32_t> newFramebuffer(pixelCount, 0xFF000000);
    std::vector<ZBufferType> newZBuffer(pixelCount, std::numeric_limits<ZBufferType>::max());
    SDL_Texture* newTexture = SDL_CreateTexture(
        renderer, SDL_PIXELFORMAT_ARGB8888, SDL_TEXTUREACCESS_STREAMING,
        static_cast<int>(renderRes.width), static_cast<int>(renderRes.height));
    if (!newTexture) {
        std::cerr << "SDL_CreateTexture failed: " << SDL_GetError() << std::endl;
        return;
    }

    SDL_DestroyTexture(texture);
    texture = newTexture;
    resolution = renderRes;
    framebuffer.swap(newFramebuffer);
    zbuffer.swap(newZBuffer);
}


void Screen::setWindowRelativeMouseMode(const bool mode) {
    SDL_SetWindowRelativeMouseMode(window, mode);
}

void Screen::setWindowType(SDL_WindowFlags flag, bool set) {
    switch(flag) {
        case SDL_WINDOW_RESIZABLE:
            SDL_SetWindowResizable(window, set);
            break;
        case SDL_WINDOW_BORDERLESS:
            SDL_SetWindowBordered(window, !set);
            break;
        case SDL_WINDOW_FULLSCREEN:
            SDL_SetWindowFullscreen(window, set);
            break;
        case SDL_WINDOW_HIDDEN:
            if(set) {
                SDL_HideWindow(window);
            } else {
                SDL_ShowWindow(window);
            }
            break;
    }
}


void Screen::clearScreen() {
    clearFramebuffer();
    printBuffer();
}

void Screen::clearFramebuffer() {
    std::fill(framebuffer.begin(),framebuffer.end(), 0xFF000000);
}

void Screen::clearFramebuffer(Bounds bounds) {
    const uint32_t left = std::min(bounds.left, resolution.width);
    const uint32_t top = std::min(bounds.top, resolution.height);
    const uint32_t right = std::min(bounds.right, resolution.width);
    const uint32_t bottom = std::min(bounds.bottom, resolution.height);
    if (left >= right || top >= bottom)
        return;

    for (uint32_t y = top; y < bottom; ++y) {
        const size_t rowStart = static_cast<size_t>(y) * resolution.width + left;
        const size_t rowEnd = rowStart + (right - left);
        std::fill(framebuffer.begin() + rowStart, framebuffer.begin() + rowEnd, 0xFF000000);
    }
}

void Screen::clearZBuffer() {
    std::fill(zbuffer.begin(), zbuffer.end(), std::numeric_limits<ZBufferType>::max());
}

void Screen::clearZBuffer(Bounds bounds) {
    const uint32_t left = std::min(bounds.left, resolution.width);
    const uint32_t top = std::min(bounds.top, resolution.height);
    const uint32_t right = std::min(bounds.right, resolution.width);
    const uint32_t bottom = std::min(bounds.bottom, resolution.height);
    if (left >= right || top >= bottom)
        return;

    for (uint32_t y = top; y < bottom; ++y) {
        const size_t rowStart = static_cast<size_t>(y) * resolution.width + left;
        const size_t rowEnd = rowStart + (right - left);
        std::fill(zbuffer.begin() + rowStart, zbuffer.begin() + rowEnd, std::numeric_limits<ZBufferType>::max());
    }
}

void Screen::fillFramebuffer(const graphics::Colour colour) {
    std::fill(framebuffer.begin(),framebuffer.end(), colour);
}


void Screen::fillFramebuffer(const graphics::Image<graphics::Colour>& image, const graphics::Region uv) {
    if (uv.width <= 0.0f || uv.height <= 0.0f ||
        image.size.width == 0 || image.size.height == 0 || image.data.empty())
        return;

    const int left = std::max(0, static_cast<int>(std::floor(uv.x * resolution.width)));
    const int top = std::max(0, static_cast<int>(std::floor(uv.y * resolution.height)));
    const int right = std::min(static_cast<int>(resolution.width),
        static_cast<int>(std::ceil((uv.x + uv.width) * resolution.width)));
    const int bottom = std::min(static_cast<int>(resolution.height),
        static_cast<int>(std::ceil((uv.y + uv.height) * resolution.height)));
    
    if (left >= right || top >= bottom)
        return;

    const float regionLeft = uv.x * resolution.width;
    const float regionTop = uv.y * resolution.height;
    const float regionWidth = uv.width * resolution.width;
    const float regionHeight = uv.height * resolution.height;
    const float maxU = std::nextafter(1.0f, 0.0f);
    const float maxV = std::nextafter(1.0f, 0.0f);

    for (int y = top; y < bottom; ++y) {
        const float v = std::clamp((y + 0.5f - regionTop) / regionHeight, 0.0f, maxV);
        const uint32_t sourceY = std::min(
            static_cast<uint32_t>(v * image.size.height), image.size.height - 1);
        for (int x = left; x < right; ++x) {
            const float u = std::clamp((x + 0.5f - regionLeft) / regionWidth, 0.0f, maxU);
            const uint32_t sourceX = std::min(
                static_cast<uint32_t>(u * image.size.width), image.size.width - 1);
            const size_t index = static_cast<size_t>(y) * resolution.width + x;
            framebuffer[index] = image.get(sourceX, sourceY);
        }
    }
}

void Screen::skybox(const graphics::Image<graphics::Colour>& image, math::Vec3 rotation) {
    if (resolution.width == 0 || resolution.height == 0 ||
        image.size.width < 4 || image.size.height < 3 ||
        image.data.size() < static_cast<size_t>(image.size.width) * image.size.height) {
        std::cerr << "Invalid skybox image: expected a complete 4x3 cross" << std::endl;
        return;
    }

    const float degreesToRadians = 3.14159265359f / 180.0f;
    rotation = rotation * degreesToRadians;
    const float sinX = std::sin(rotation.x), cosX = std::cos(rotation.x);
    const float sinY = std::sin(rotation.y), cosY = std::cos(rotation.y);
    const float sinZ = std::sin(rotation.z), cosZ = std::cos(rotation.z);
    const float maxUv = std::nextafter(1.0f, 0.0f);
    struct FaceTile { uint32_t left, top, width, height; };
    std::array<FaceTile, 12> tiles{};
    for (uint32_t faceY = 0; faceY < 3; ++faceY) {
        for (uint32_t faceX = 0; faceX < 4; ++faceX) {
            auto& tile = tiles[faceY * 4 + faceX];
            tile.left = static_cast<uint32_t>(static_cast<uint64_t>(faceX) * image.size.width / 4);
            tile.top = static_cast<uint32_t>(static_cast<uint64_t>(faceY) * image.size.height / 3);
            const uint32_t right = static_cast<uint32_t>(
                static_cast<uint64_t>(faceX + 1) * image.size.width / 4);
            const uint32_t bottom = static_cast<uint32_t>(
                static_cast<uint64_t>(faceY + 1) * image.size.height / 3);
            tile.width = right - tile.left;
            tile.height = bottom - tile.top;
        }
    }

    const auto rotateRay = [&](math::Vec3 ray) {
        ray = math::Vec3(ray.x, ray.y * cosX - ray.z * sinX, ray.y * sinX + ray.z * cosX);
        ray = math::Vec3(ray.x * cosY + ray.z * sinY, ray.y, -ray.x * sinY + ray.z * cosY);
        return math::Vec3(ray.x * cosZ - ray.y * sinZ, ray.x * sinZ + ray.y * cosZ, ray.z);
    };
    const math::Vec3 xBasis = rotateRay(math::Vec3(1, 0, 0));
    const math::Vec3 yBasis = rotateRay(math::Vec3(0, 1, 0));
    const math::Vec3 zBasis = rotateRay(math::Vec3(0, 0, 1));
    const math::Vec3 rayStep = xBasis * (2.0f / resolution.width);
    const math::Vec3 firstColumn = xBasis * (-1.0f + 1.0f / resolution.width) - zBasis;

    for (uint32_t y = 0; y < resolution.height; ++y) {
        const float rayY = 1.0f - 2.0f * (y + 0.5f) / resolution.height;
        math::Vec3 ray = firstColumn + yBasis * rayY;
        for (uint32_t x = 0; x < resolution.width; ++x) {
            const float ax = std::abs(ray.x), ay = std::abs(ray.y), az = std::abs(ray.z);
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

            const FaceTile& tile = tiles[faceY * 4 + faceX];
            const float faceU = std::clamp((u + 1.0f) * 0.5f, 0.0f, maxUv);
            const float faceV = std::clamp((v + 1.0f) * 0.5f, 0.0f, maxUv);
            const uint32_t localX = std::min(static_cast<uint32_t>(faceU * tile.width), tile.width - 1);
            const uint32_t localY = std::min(static_cast<uint32_t>(faceV * tile.height), tile.height - 1);
            const uint32_t sourceX = tile.left + localX;
            const uint32_t sourceY = tile.top + localY;
            framebuffer[static_cast<size_t>(y) * resolution.width + x] = image.get(sourceX, sourceY);
            ray += rayStep;
        }
    }
}

void Screen::printBuffer()
{
    // paste buffer to texture and upload to renderer
    SDL_RenderClear(renderer);

    if (!SDL_UpdateTexture(texture, nullptr, framebuffer.data(), resolution.width * sizeof(uint32_t))) {
        std::cerr << "SDL_UpdateTexture failed: " << SDL_GetError() << std::endl;
        return;
    }

    if (!SDL_RenderTexture(renderer, texture, nullptr, nullptr)) {
        std::cerr << "SDL_RenderTexture failed: " << SDL_GetError() << std::endl;
        return;
    }

    SDL_RenderPresent(renderer);
}








std::vector<uint32_t>* Screen::getFramebuffer() {
    return &framebuffer;
}

std::vector<ZBufferType>* Screen::getZBuffer() {
    return &zbuffer;
}


void Screen::fixTexture() {
    SDL_Texture* newTexture = SDL_CreateTexture(
        renderer, SDL_PIXELFORMAT_ARGB8888, SDL_TEXTUREACCESS_STREAMING,
        static_cast<int>(resolution.width), static_cast<int>(resolution.height));
    if (!newTexture) {
        std::cerr << "SDL_CreateTexture failed: " << SDL_GetError() << std::endl;
        return;
    }

    SDL_DestroyTexture(texture);
    texture = newTexture;
}


Viewport Screen::tieViewport(graphics::Region tile, graphics::Region area) {
    Viewport viewport{};

    viewport.screen = this;
    viewport.tile = tile;
    viewport.area = area;

    viewport.recalculateBounds();
    return viewport;
}




bool Viewport::triangleOutside(float focalLength, const math::Vec3& a, const math::Vec3& b, const math::Vec3& c)
{
    const float areaWidth = static_cast<float>(areaBounds.right - areaBounds.left);
    const float areaHeight = static_cast<float>(areaBounds.bottom - areaBounds.top);
    const float xScale = 1.0f / focalLength;
    const float yScale = 1.0f / focalLength;

    const float tileLeft = static_cast<float>(tileBounds.left - areaBounds.left);
    const float tileTop = static_cast<float>(tileBounds.top - areaBounds.top);
    const float tileRight = static_cast<float>(tileBounds.right - areaBounds.left);
    const float tileBottom = static_cast<float>(tileBounds.bottom - areaBounds.top);

    const float left = 2.0f * tileLeft / areaWidth - 1.0f;
    const float right = 2.0f * tileRight / areaWidth - 1.0f;
    const float top = 1.0f - 2.0f * tileTop / areaHeight;
    const float bottom = 1.0f - 2.0f * tileBottom / areaHeight;

    const float depthA = -a.z;
    const float depthB = -b.z;
    const float depthC = -c.z;

    return (a.x < left * depthA * xScale && b.x < left * depthB * xScale && c.x < left * depthC * xScale) ||
           (a.x > right * depthA * xScale && b.x > right * depthB * xScale && c.x > right * depthC * xScale) ||
           (a.y > top * depthA * yScale && b.y > top * depthB * yScale && c.y > top * depthC * yScale) ||
           (a.y < bottom * depthA * yScale && b.y < bottom * depthB * yScale && c.y < bottom * depthC * yScale);
}

void Viewport::recalculateBounds() 
{
    if (!screen)
        return;

    const auto res = screen->getRenderRes();
    const int areaWidth = static_cast<int>(std::lround(area.width * res.width));
    const int areaHeight = static_cast<int>(std::lround(area.height * res.height));

    areaBounds.left = static_cast<uint32_t>(std::lround(area.x * res.width));
    areaBounds.top = static_cast<uint32_t>(std::lround(area.y * res.height));
    areaBounds.right = areaBounds.left + static_cast<uint32_t>(areaWidth);
    areaBounds.bottom = areaBounds.top + static_cast<uint32_t>(areaHeight);

    tileBounds.left = areaBounds.left + static_cast<uint32_t>(std::floor(tile.x * areaWidth));
    tileBounds.top = areaBounds.top + static_cast<uint32_t>(std::floor(tile.y * areaHeight));
    tileBounds.right = areaBounds.left + static_cast<uint32_t>(std::ceil((tile.x + tile.width) * areaWidth));
    tileBounds.bottom = areaBounds.top + static_cast<uint32_t>(std::ceil((tile.y + tile.height) * areaHeight));
}



};