#include "Screen.hpp"

namespace hybriddisplay::display {

Screen::Screen(const graphics::Resolution& resolution) : resolution(resolution) {
    framebuffer.resize(resolution.width * resolution.height);
    zbuffer.resize(resolution.width * resolution.height, std::numeric_limits<float>::infinity());
}

Viewport Screen::tieViewport(float x, float y, float width, float height) {
    Viewport viewport;
    viewport.area.x = x;
    viewport.area.y = y;
    viewport.area.width = width;
    viewport.area.height = height;
    
    viewport.framebuffer = &framebuffer;
    viewport.zbuffer = &zbuffer;
    return viewport;
}


};