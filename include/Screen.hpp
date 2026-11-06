#ifndef SCREEN_HPP
#define SCREEN_HPP

#include "SDL3/SDL.h"
#include "Material.hpp"

namespace hybriddisplay::display {

struct Bounds {
    uint32_t left;
    uint32_t top;
    uint32_t right;
    uint32_t bottom;
};

struct Viewport;

class Screen {
private:
    SDL_Renderer* renderer = nullptr;
    SDL_Window* window = nullptr;
    SDL_Texture* texture = nullptr;
    
    graphics::Resolution resolution{0, 0};
    std::vector<uint32_t> framebuffer; // pixel buffer for the screen, in ARGB8888 format
    std::vector<float> zbuffer; // depth buffer for z-buffering

public:
    
    Screen(const graphics::Resolution& window, const graphics::Resolution& render);
    Screen(const graphics::Resolution& window);
    ~Screen();


    graphics::Resolution getWindowRes();
    graphics::Resolution getRenderRes();
    void resizeWindow(const graphics::Resolution& windowRes);
    void resizeRender(const graphics::Resolution& renderRes);
    void setWindowRelativeMouseMode(const bool mode); // consumes mouse pointer
    void setWindowType(SDL_WindowFlags flags, bool set);

    void clearFramebuffer(); // clears the framebuffer
    void clearFramebuffer(Bounds bounds); // clears the framebuffer within pixel bounds
    void clearFramebuffer(graphics::Region bounds); // clears the framebuffer within normalized bounds
    void clearZBuffer(); // clears the zbuffer
    void clearZBuffer(Bounds bounds); // clears the zbuffer within pixel bounds
    void clearZBuffer(graphics::Region bounds); // clears the zbuffer within normalized bounds
    void clearScreen(); // clears framebuffer and screen
    
    void fillFramebuffer(const graphics::Colour colour);
    void fillFramebuffer(std::shared_ptr<const graphics::Image<graphics::Colour>> image, const graphics::Region uv);
    void printBuffer();
    
    std::vector<uint32_t>* getFramebuffer();
    std::vector<float>* getZBuffer();
    
    void fixTexture();

    Viewport tieViewport(graphics::Region tile, graphics::Region area); // overload for tying a viewport using normalized coordinates (0.0 to 1.0)
};

struct Viewport {
    Screen* screen;

    graphics::Region tile; // normalized part of the viewport that is being rendered to
    graphics::Region area; // normalized position and size in the final framebuffer

    Bounds tileBounds;
    Bounds areaBounds;

    graphics::Resolution resolution() { 
        return screen->getRenderRes();
    };

    uint32_t getWidth() {return screen->getRenderRes().width; } // returns render width
    uint32_t getHeight() {return screen->getRenderRes().height; } // returns render height

    std::vector<uint32_t>* framebuffer() {
        return screen->getFramebuffer();
    };

    std::vector<float>* zbuffer() const {
        return screen->getZBuffer();
    }

    bool triangleOutside(float focalLength, const math::Vec3& a, const math::Vec3& b, const math::Vec3& c);
    void recalculateBounds();
};


};

#endif