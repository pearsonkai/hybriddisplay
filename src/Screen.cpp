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

    window = SDL_CreateWindow("My SDL3 Window", resolution.width, resolution.height, SDL_WINDOW_RESIZABLE);
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




Viewport Screen::tieViewport(float x, float y, float width, float height) {
    Viewport viewport;
    viewport.area.x = x * resolution.width;
    viewport.area.y = y * resolution.height;
    viewport.area.width = width * resolution.width;
    viewport.area.height = height * resolution.height;
    viewport.framebufferWidth = resolution.width;

    viewport.framebuffer = &framebuffer;
    viewport.zbuffer = &zbuffer;
    return viewport;
}


};