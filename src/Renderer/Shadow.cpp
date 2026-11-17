#include "Renderer.hpp"
#include <algorithm>
#include <cmath>

namespace hybriddisplay::rendering {

graphics::Image<float> Renderer::shadow(display::Screen& screen, const Camera& camera, const geometry::World& world) {
    screen.clearZBuffer();
    std::vector<display::Viewport> temp;
    temp.push_back(screen.tieViewport({0,0,1,1},{0,0,1,1}));

    rasterize(temp, camera, world);
    return graphics::Image<float>(screen.getZBuffer(), screen.getRenderRes());
}

};