#include "Renderer.hpp"
#include <algorithm>
#include <cmath>

namespace hybriddisplay::rendering {

void Renderer::shadow(display::Screen& screen, const Camera& camera, const geometry::World& world) {
    constexpr graphics::Resolution lightmapRes = {2000,2000};
    graphics::Resolution tempRes = screen.getRenderRes();
    
    Culling tempStyle = cullingStyle; 
    cullingStyle = frontface;
    
    screen.clearZBuffer();
    screen.resizeRender(lightmapRes);
    
    std::vector<display::Viewport> temp;
    temp.push_back(screen.tieViewport({0,0,1,1},{0,0,1,1}));

    if(worldShadow != nullptr) {
        delete worldShadow;
    }
    worldShadow = new Shadow;
    worldShadow->source = Camera(camera);

    rasterize(temp, camera, world);
    cullingStyle = tempStyle;
    
    
    worldShadow->cache = graphics::Image<float>(screen.getZBuffer(), screen.getRenderRes());
    
    screen.resizeRender(tempRes);
    worldShadow->ready = true;
}

};