#include <cmath>
#include <algorithm>
#include <cstring>
#include <iostream>
#include <limits>
#include "Screen.hpp"

namespace hybriddisplay::display {

Screen::Screen(const graphics::Resolution& resolution) : resolution(resolution) {
    framebuffer.resize(resolution.width * resolution.height);
    zbuffer.resize(resolution.width * resolution.height, std::numeric_limits<ZBufferType>::max());

    window = SDL_CreateWindow("hybriddisplay", resolution.width, resolution.height, SDL_WINDOW_RESIZABLE);
    if (!window)
    {
        std::cerr << "SDL_CreateWindow failed: " << SDL_GetError() << std::endl;
    }

    renderer = SDL_CreateRenderer(window, NULL);
    if (!renderer)
    {
        std::cerr << "SDL_CreateRenderer failed: " << SDL_GetError() << std::endl;
    }

    texture = SDL_CreateTexture(renderer, SDL_PIXELFORMAT_ARGB8888, SDL_TEXTUREACCESS_STREAMING, resolution.width, resolution.height);
    if (!texture)
    {
        std::cerr << "SDL_CreateTexture failed: " << SDL_GetError() << std::endl;
    }
}

Screen::~Screen() {
    SDL_DestroyTexture(texture);
    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);
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

    if (!SDL_UpdateTexture(texture, nullptr, framebuffer.data(), resolution.width * sizeof(uint32_t)))
    {
        std::cerr << "SDL_UpdateTexture failed: " << SDL_GetError() << std::endl;
        return;
    }

    if (!SDL_RenderTexture(renderer, texture, nullptr, nullptr))
    {
        std::cerr << "SDL_RenderTexture failed: " << SDL_GetError() << std::endl;
        return;
    }

    SDL_RenderPresent(renderer);
}




Viewport Screen::tieViewport(graphics::Region tile, graphics::Region area) {
    Viewport viewport;
    viewport.area = area;
    viewport.tile = tile;
    viewport.resolution = resolution;

    const int areaWidth = static_cast<int>(std::lround(area.width * resolution.width));
    const int areaHeight = static_cast<int>(std::lround(area.height * resolution.height));
    viewport.areaBounds.left = static_cast<int>(std::lround(area.x * resolution.width));
    viewport.areaBounds.top = static_cast<int>(std::lround(area.y * resolution.height));
    viewport.areaBounds.right = viewport.areaBounds.left + areaWidth;
    viewport.areaBounds.bottom = viewport.areaBounds.top + areaHeight;
    viewport.tileBounds.left = static_cast<int>(std::floor(tile.x * areaWidth));
    viewport.tileBounds.top = static_cast<int>(std::floor(tile.y * areaHeight));
    viewport.tileBounds.right = static_cast<int>(std::ceil((tile.x + tile.width) * areaWidth));
    viewport.tileBounds.bottom = static_cast<int>(std::ceil((tile.y + tile.height) * areaHeight));

    viewport.framebuffer = &framebuffer;
    viewport.zbuffer = &zbuffer;
    return viewport;
}


};