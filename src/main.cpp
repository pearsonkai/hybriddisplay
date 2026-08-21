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
        std::cout << ".dll files not found. Move them to .exe directory" << std::endl;
    }

    
    // std::unordered_map<std::string, std::unique_ptr<rendering::Material>> material_library;
    // std::vector<std::unique_ptr<geometry::Mesh>> mesh_pile;

    //Screen screen = new Screen(RESOLUTION);
    
    while(true)
    {
        break;
    };


    return 0;
}