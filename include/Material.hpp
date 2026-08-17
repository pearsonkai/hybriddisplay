#ifndef MATERIAL_HPP
#define MATERIAL_HPP

#include <vector>
#include <filesystem>
#include <string>
#include <cstdint>
#include <algorithm>
#include "Vec3.hpp"

namespace fs = std::filesystem;

namespace hybriddisplay::rendering {

struct Region
{
    uint32_t x, y;
    uint32_t width, height;
};

struct Resolution
{
    uint32_t width;
    uint32_t height;
}; // usable for colour and normal map sizes, camera viewport sizes, screen size total, etc...

template<typename T>
struct Image {
    Resolution size;
    std::vector<T> data;

    Image(uint32_t width, uint32_t height) : size{width, height}, data(width * height){}

    const T& get(uint32_t x, uint32_t y) const
    {
        return data[y * size.width + x];
    }
};

struct Colour {
    uint8_t r, g, b, a;

    Colour(uint8_t r = 0, uint8_t g = 0, uint8_t b = 0, uint8_t a = 255);
    uint32_t convertRGBA() const; // converts the colour to a 32-bit RGBA value
};

using Greyscale = uint8_t; // for specular maps, where the value represents the intensity of the specular reflection

class Material {
private:
    std::string name;

    Image<Colour> textureMap;
    Image<math::Vec3> normalMap;
    Image<Greyscale> specularMap; // optional specular map for more advanced lighting effects

    Colour ambient; // (Ka) ambient colour of the material
    Colour diffuse; // (Kd) base colour of the material
    Colour specular; // (Ks) specular colour of the material 
    float reflectiveness;

public:
    Material(const Colour& diffuse, const Colour& specular, const Colour& ambient = Colour(0, 0, 0, 255), float reflectiveness = 0.5f);

    void loadTextureMap(const Image<Colour>& image);
    void loadNormalMap(const Image<Colour>& image);
    void loadSpecularMap(const Image<Colour>& image);

    static float wrap(float uv);
    static float bound(float uv); 

    Colour sampleTexture(float u, float v) const; // returns the colour at the UV coord
    math::Vec3 sampleNormal(float u, float v) const; // returns the converted colour to normal at the UV coord
    Greyscale sampleSpecular(float u, float v) const; // returns the specular intensity at the UV coord

    static Image<Colour> loadPNG(const fs::path& filePath);
    static math::Vec3 colourToVec3(const Colour& colour);
    static Greyscale colourToGreyscale(const Colour& colour);
};

const Colour MAGENTA = Colour(255, 0, 255, 255); // default colour for missing textures


};

#endif