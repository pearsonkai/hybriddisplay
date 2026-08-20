#ifndef SCREEN_HPP
#define SCREEN_HPP

#include "SDL3/SDL.h"
#include "Material.hpp"

namespace hybriddisplay::display {
    
struct Viewport {
    graphics::Region area;

    std::vector<rendering::Colour>* framebuffer;
    std::vector<float>* zbuffer; // depth buffer for z-buffering
};

class Screen {
private:
    SDL_Renderer* renderer;
    SDL_Window* window;
    SDL_Texture* texture;
    
    graphics::Resolution resolution;
    std::vector<graphics::Colour> framebuffer; // pixel buffer for the screen
    std::vector<float> zbuffer; // depth buffer for z-buffering

public:
    
    Screen(const graphics::Resolution& resolution);

    void clearScreen()
    void clearFramebuffer()
    void clearZBuffer()
    
    void printBuffer();

    // presentFrame()

    Viewport tieViewport(float x, float y, float width, float height); // overload for tying a viewport using normalized coordinates (0.0 to 1.0)
};

};

#endif