#ifndef SCREEN_HPP
#define SCREEN_HPP

#include "Material.hpp"
// #include "SDL/SDL3.h" <-- This is a placeholder for the SDL3 header file which will be added once the SDL3 library is integrated into the project

namespace hybriddisplay::display {

struct Viewport {
    graphics::Resolution start; // starting coordinates of the viewport
    graphics::Resolution size; // width and height of the viewport

    std::vector<graphics::Colour>* framebuffer;
    std::vector<float>* zbuffer; // depth buffer for z-buffering
};

class Screen {
private:
    graphics::Resolution resolution;
    std::vector<graphics::Colour> framebufer; // pixel buffer for the screen
    std::vector<float> zbuffer; // depth buffer for z-buffering

public:
    
    Screen(const graphics::Resolution& resolution);

    // clearScreen()
    // clearFramebuffer()
    // clearZBuffer()

    // presentFrame()

    Viewport tieViewport(const graphics::Resolution& start, const graphics::Resolution& size);
    Viewport tieViewport(float startX, float startY, float endX, float endY); // overload for tying a viewport using normalized coordinates (0.0 to 1.0)
};

};

#endif