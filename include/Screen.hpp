#ifndef SCREEN_HPP
#define SCREEN_HPP

#include "Material.hpp"
// #include "SDL/SDL3.h" <-- This is a placeholder for the SDL3 header file which will be added once the SDL3 library is integrated into the project

namespace hybriddisplay::display {
    
struct Viewport {
    rendering::Region area;

    std::vector<rendering::Colour>* framebuffer;
    std::vector<float>* zbuffer; // depth buffer for z-buffering
};

class Screen {
private:
    rendering::Resolution resolution;
    std::vector<rendering::Colour> framebuffer; // pixel buffer for the screen
    std::vector<float> zbuffer; // depth buffer for z-buffering

public:
    
    Screen(const rendering::Resolution& resolution);

    void clearScreen()
    void clearFramebuffer()
    void clearZBuffer()


    // presentFrame()

    Viewport tieViewport(float x, float y, float width, float height); // overload for tying a viewport using normalized coordinates (0.0 to 1.0)
};

};

#endif