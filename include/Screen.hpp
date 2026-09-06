#ifndef SCREEN_HPP
#define SCREEN_HPP

#include <cstring>
#include "SDL3/SDL.h"
#include "Material.hpp"

namespace hybriddisplay::display {

using ZBufferType  = float;


struct Viewport {
    graphics::Region tile; // normalized part of the viewport that is being rendered to
    graphics::Region area; // normalized position and size in the final framebuffer

    graphics::Resolution resolution; // the resolution of the framebuffer and zbuffer for this viewport

    std::vector<uint32_t>* framebuffer;
    std::vector<ZBufferType>* zbuffer; // depth buffer for z-buffering
};

class Screen {
private:
    SDL_Renderer* renderer;
    SDL_Window* window;
    SDL_Texture* texture;
    
    graphics::Resolution resolution;
    std::vector<uint32_t> framebuffer; // pixel buffer for the screen, in ARGB8888 format
    std::vector<ZBufferType> zbuffer; // depth buffer for z-buffering

public:
    
    Screen(const graphics::Resolution& resolution);
    ~Screen();

    void clearScreen();
    void clearFramebuffer();
    void clearZBuffer();
    void printBuffer();

    // presentFrame()

    Viewport tieViewport(graphics::Region tile, graphics::Region area); // overload for tying a viewport using normalized coordinates (0.0 to 1.0)
};

};

#endif