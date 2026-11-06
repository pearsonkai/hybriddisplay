#include "Renderer.hpp"
#include <algorithm>
#include <array>
#include <cmath>

namespace hybriddisplay::rendering {

struct Ray {
    math::Vec3 origin;
    math::Vec3 direction;

    math::Vec3 at(float distance) const {
        return origin + direction.normalize() * distance;
    }
};


void raytrace(std::vector<display::Viewport>& viewports, const Camera& camera, const geometry::World& world) {
    for(display::Viewport& viewport : viewports) {
        //const uint32_t width = viewport.resolution().width;
        //const uint32_t height = viewport.resolution().height;

        for(int i = 0; i < width; i++) {
            for(int j = 0; j < height; j++) {
                graphics::Colour pixel;

            }
        }


        
    }
}
    

};