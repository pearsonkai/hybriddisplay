#ifndef SCREEN_HPP
#define SCREEN_HPP

#include <cstring>
#include "SDL3/SDL.h"
#include "Material.hpp"

namespace hybriddisplay::display {

using ZBufferType  = float;
struct Viewport;

class Screen {
private:
    SDL_Renderer* renderer;
    SDL_Window* window;
    SDL_Texture* texture;
    
    graphics::Resolution resolution;
    std::vector<uint32_t> framebuffer; // pixel buffer for the screen, in ARGB8888 format
    std::vector<ZBufferType> zbuffer; // depth buffer for z-buffering

public:
    
    Screen(const graphics::Resolution& window);
    Screen(const graphics::Resolution& window, const graphics::Resolution& render);
    ~Screen();


    graphics::Resolution getWindowRes();
    graphics::Resolution getRenderRes();
    void resizeWindow(const graphics::Resolution& windowRes);
    void resizeRender(const graphics::Resolution& renderRes);

    void clearScreen();
    void clearFramebuffer();
    void clearZBuffer();
    void printBuffer();

    std::vector<uint32_t>* getFramebuffer();
    std::vector<ZBufferType>* getZBuffer();
    
    void fixTexture();

    Viewport tieViewport(graphics::Region tile, graphics::Region area); // overload for tying a viewport using normalized coordinates (0.0 to 1.0)
};

struct Viewport {
    struct Bounds {
        uint32_t left;
        uint32_t top;
        uint32_t right;
        uint32_t bottom;
    };

    Screen* screen;

    graphics::Region tile; // normalized part of the viewport that is being rendered to
    graphics::Region area; // normalized position and size in the final framebuffer

    Bounds tileBounds;
    Bounds areaBounds;

    //graphics::Resolution resolution;
    graphics::Resolution resolution() {
        return screen->getRenderRes();
    };

    std::vector<uint32_t>* framebuffer() {
        return screen->getFramebuffer();
    };

    std::vector<ZBufferType>* zbuffer() const {
        return screen->getZBuffer();
    }

    bool triangleOutside(float focalLength, const math::Vec3& a, const math::Vec3& b, const math::Vec3& c);
    void recalculateBounds();
};


};

#endif