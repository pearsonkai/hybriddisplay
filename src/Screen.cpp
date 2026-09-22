#include <cmath>
#include <algorithm>
#include <cstring>
#include <iostream>
#include <limits>
#include "Screen.hpp"

namespace hybriddisplay::display {

Screen::Screen(const graphics::Resolution& windowRes, const graphics::Resolution& renderRes){
    
    resolution = renderRes;
    framebuffer.resize(resolution.width * resolution.height);
    zbuffer.resize(resolution.width * resolution.height, std::numeric_limits<ZBufferType>::max());

    window = SDL_CreateWindow("hybriddisplay", windowRes.width, windowRes.height, SDL_WINDOW_RESIZABLE);
    if (!window) {
        std::cerr << "SDL_CreateWindow failed: " << SDL_GetError() << std::endl;
    }

    renderer = SDL_CreateRenderer(window, NULL);
    if (!renderer) {
               std::cerr << "SDL_CreateRenderer failed: " << SDL_GetError() << std::endl;
    }

    texture = SDL_CreateTexture(renderer, SDL_PIXELFORMAT_ARGB8888, SDL_TEXTUREACCESS_STREAMING, resolution.width, resolution.height);
    if (!texture) {
        std::cerr << "SDL_CreateTexture failed: " << SDL_GetError() << std::endl;
    }
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
    resolution = renderRes;
    uint64_t indexSize = renderRes.width * renderRes.height;
    
    fixTexture();

    framebuffer.resize(indexSize);
    zbuffer.resize(indexSize);
    clearFramebuffer();
    clearZBuffer();
}




void Screen::clearScreen() {
    clearFramebuffer();
    printBuffer();
}

void Screen::clearFramebuffer() {
    std::fill(framebuffer.begin(),framebuffer.end(), 0xFF000000);
}

void Screen::clearZBuffer() {
    std::fill(zbuffer.begin(), zbuffer.end(), std::numeric_limits<ZBufferType>::max());
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
    if(texture) {
        SDL_DestroyTexture(texture);
    }

    texture = SDL_CreateTexture(renderer, SDL_PIXELFORMAT_ARGB8888, SDL_TEXTUREACCESS_STREAMING, resolution.width, resolution.height);
    if (!texture) {
        std::cerr << "SDL_CreateTexture failed: " << SDL_GetError() << std::endl;
    }
}


Viewport& Screen::tieViewport(graphics::Region tile, graphics::Region area) {
    static Viewport viewport{};

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
    const float aspect = areaWidth / std::max(areaHeight, 1.0f);
    const float xScale = aspect / focalLength;
    const float yScale = 1.0f / focalLength;

    const float left = 2.0f * tileBounds.left / areaWidth - 1.0f;
    const float right = 2.0f * tileBounds.right / areaWidth - 1.0f;
    const float top = 1.0f - 2.0f * tileBounds.top / areaHeight;
    const float bottom = 1.0f - 2.0f * tileBounds.bottom / areaHeight;

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

    tileBounds.left = static_cast<uint32_t>(std::floor(tile.x * areaWidth));
    tileBounds.top = static_cast<uint32_t>(std::floor(tile.y * areaHeight));
    tileBounds.right = static_cast<uint32_t>(std::ceil((tile.x + tile.width) * areaWidth));
    tileBounds.bottom = static_cast<uint32_t>(std::ceil((tile.y + tile.height) * areaHeight));
}



};