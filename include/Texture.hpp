#ifndef TEXTURE_HPP
#define TEXTURE_HPP

#include <vector>
#include <filesystem>
#include "Vec3.hpp"

using namespace fs = std::filesystem;

namespace hybriddisplay::graphics {

struct Resolution
{
    uint32_t width;
    uint32_t height;
}; // usable for colour and normal map sizes, camera viewport sizes, screen size total, etc...

struct Colour {
    float r, g, b, a;

    Colour(float r = 0, float g = 0, float b = 0, float a = 1);
}; 


class Texture {
private:
    Resolution colourMapSize;
    Resolution normalMapSize;

    std::vector<Colour> colourMap;
    std::vector<Colour> normalMap;
        
public:
    Texture(uint_32t width, uint_32t height);

    void loadColourMap(const fs::path& filePath);
    void loadNormalMap(const fs::path& filePath);

    Colour sampleColour(float u, float v) const; // returns the colour at the UV coord
    Vec3 sampleNormal(float u, float v) const; // returns the converted colour to normal at the UV coord

    static std::vector<Colour> loadPNG(const fs::path& filePath) const;
    static Vec3 convertColourToNormal(const Colour& colour) const;
};

};

#endif