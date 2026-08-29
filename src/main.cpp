#include <iostream>
#include <unordered_map>

#include <iostream>
#include <windows.h>
#include "Renderer.hpp"

using namespace hybriddisplay;

const graphics::Resolution RESOLUTION = {800,600};

int main()
{
    if ( !SetDllDirectoryA("lib")) {
        std::cout << ".dll files not found. Move them to the .exe directory" << std::endl;
    }

    
    // std::unordered_map<std::string, std::unique_ptr<rendering::Material>> material_library;
    // std::vector<std::unique_ptr<geometry::Mesh>> mesh_pile;

    display::Screen screen = display::Screen(RESOLUTION);
    display::Viewport singleViewport = screen.tieViewport(0,0,1,1);

    rendering::Renderer renderer = rendering::Renderer();
    rendering::Camera camera = rendering::Camera();
    

    geometry::World mainWorld = geometry::World();
    //geometry::Mesh cube = geometry::Mesh();
    

    while(true)
    {
        screen.clearFramebuffer();
        screen.clearZBuffer();

        renderer.wireframe(singleViewport,camera,mainWorld);
        screen.printBuffer();
    };


    return 0;
}