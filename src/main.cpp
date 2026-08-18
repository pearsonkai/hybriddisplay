#include <iostream>
#include <unordered_map>

#include "Camera.hpp"

using namespace hybriddisplay;

int main()
{
    std::unordered_map<std::string, std::unique_ptr<rendering::Material>> material_library;
    std::vector<std::unique_ptr<geometry::Mesh>> mesh_pile;

    std::cout << "Hello, World!" << std::endl;
    return 0;
}