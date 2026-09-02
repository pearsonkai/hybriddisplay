#include <iostream>
#include <unordered_map>

#include <iostream>
#include <windows.h>
#include "Renderer.hpp"

using namespace hybriddisplay;

const graphics::Resolution RESOLUTION = {800,600};

int main()
{
    char exePath[MAX_PATH];

    GetModuleFileNameA(nullptr, exePath, MAX_PATH);

    std::string path(exePath);
    path = path.substr(0, path.find_last_of("\\/"));

    std::string libPath = path + "\\lib";

    if (!SetDllDirectoryA(libPath.c_str())) {
        std::cerr << "Failed to set DLL directory. Error: "
                  << GetLastError() << '\n';
        return 1;
    }

    
    // std::unordered_map<std::string, std::unique_ptr<rendering::Material>> material_library;
    // std::vector<std::unique_ptr<geometry::Mesh>> mesh_pile;

    display::Screen screen = display::Screen(RESOLUTION);
    display::Viewport singleViewport = screen.tieViewport(0,0,1,1);

    rendering::Renderer renderer = rendering::Renderer();
    rendering::Camera camera = rendering::Camera();
    

    geometry::World mainWorld = geometry::World();
    //geometry::Mesh cube = geometry::Mesh();
    
    bool running = true;
    SDL_Event event;
    bool keys[SDL_SCANCODE_COUNT] = {};

    while(true)
    {
        if (!running || keys[SDL_SCANCODE_ESCAPE])
        {
            break;
        }

        while (SDL_PollEvent(&event))
        {
            if (event.type == SDL_EVENT_QUIT)
            {
                running = false;
            }

            if (event.type == SDL_EVENT_KEY_DOWN && !event.key.repeat)
            {
                keys[event.key.scancode] = true;
            }

            if (event.type == SDL_EVENT_KEY_UP)
            {
                keys[event.key.scancode] = false;
            }
        }

        screen.clearFramebuffer();
        screen.clearZBuffer();

        renderer.wireframe(singleViewport,camera,mainWorld);
        screen.printBuffer();
    };


    return 0;
}