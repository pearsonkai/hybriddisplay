#include "Screen.hpp"

namespace hybriddisplay::display {

Screen::Screen(const graphics::Resolution& resolution) : resolution(resolution) {
    framebufer.resize(resolution.width * resolution.height);
    zbuffer.resize(resolution.width * resolution.height, std::numeric_limits<float>::infinity());
}

Viewport Screen::tieViewport(const graphics::Resolution& start, const graphics::Resolution& size) {
    Viewport viewport;
    viewport.start = start;
    viewport.size = size;
    
    viewport.framebuffer = &framebufer;
    viewport.zbuffer = &zbuffer;
    return viewport;
}

Viewport Screen::tieViewport(float startX, float startY, float endX, float endY) {
    graphics::Resolution start{static_cast<uint32_t>(startX * resolution.width), static_cast<uint32_t>(startY * resolution.height)};
    graphics::Resolution size{static_cast<uint32_t>((endX - startX) * resolution.width), static_cast<uint32_t>((endY - startY) * resolution.height)};
    return tieViewport(start, size);
}



};