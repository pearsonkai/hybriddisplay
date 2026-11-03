#ifndef MATERIAL_HPP
#define MATERIAL_HPP

#include <vector>
#include <filesystem>
#include <string>
#include <cstdint>
#include <cstddef>
#include <memory>
#include "Vec3.hpp"

namespace fs = std::filesystem;

namespace hybriddisplay::graphics {

struct Region {
    float x, y;
    float width, height;
}; // uses normalized coordinates to describe a rectangular region of a texture or screen

struct Resolution {
    uint32_t width, height;
}; // usable for colour and normal map sizes, camera viewport sizes, screen size total, etc...

using Greyscale = uint8_t;

struct Colour {
    uint8_t r, g, b, a;

    Colour(uint8_t r = 0, uint8_t g = 0, uint8_t b = 0, uint8_t a = 255);
    
    Colour(uint32_t AARRGGBB);
    operator uint32_t() const;
    operator Greyscale() const;
    
    Colour(math::Vec3 vec3);
    operator math::Vec3() const;

    Colour layer(const Colour& top);
    static Colour blend(const Colour& topLeft, const Colour& topRight, const Colour& bottomLeft, const Colour& bottomRight, float u = 0.5f, float v = 0.5f);
};

template<typename T>
struct Image {
    Resolution size;
    std::vector<T> data;

    Image(const fs::path& filePath);
    Image(uint32_t width = 1, uint32_t height = 1) : size{width, height}, data(width * height) {}
    static std::shared_ptr<const Image<T>> load(const fs::path& filePath);

    const T& get(uint32_t x, uint32_t y) const {
        return data[static_cast<std::size_t>(y) * size.width + x];
    }
    const T& sample(float u, float v) const;
};


template<>
Image<Colour>::Image(const fs::path& filePath);

template<>
Image<math::Vec3>::Image(const fs::path& filePath);

template<>
Image<Greyscale>::Image(const fs::path& filePath);

template<>
std::shared_ptr<const Image<Colour>> Image<Colour>::load(const fs::path& filePath);

template<>
std::shared_ptr<const Image<math::Vec3>> Image<math::Vec3>::load(const fs::path& filePath);

template<>
std::shared_ptr<const Image<Greyscale>> Image<Greyscale>::load(const fs::path& filePath);

/*
namespace lighting { 
    enum Type { 
        none,
        face,
        normal_face,
        normal_map
    };

    struct Source {
        float intenisty;
    };
};*/


class Material {
private:
    std::string name;

    std::shared_ptr<const Image<Colour>> textureMap;
    std::shared_ptr<const Image<math::Vec3>> normalMap;
    std::shared_ptr<const Image<Greyscale>> specularMap; // optional specular map for more advanced lighting effects

    Colour ambient; // (Ka) ambient colour of the material
    Colour diffuse; // (Kd) base colour of the material
    Colour specular; // (Ks) specular colour of the material 
    float reflectiveness;

public:
    Material();
    Material(const Colour& _diffuse, const Colour& _specular, const Colour& _ambient, float _reflectiveness, std::shared_ptr<const Image<Colour>> image, std::shared_ptr<const Image<math::Vec3>> normal);
    
    
    void loadTextureMap(std::shared_ptr<const Image<Colour>> image);
    void loadNormalMap(std::shared_ptr<const Image<math::Vec3>> image);
    void loadSpecularMap(std::shared_ptr<const Image<Greyscale>> image);

    static float wrap(float uv);
    static float bound(float uv); 

    Colour sampleTexture(float u, float v) const; // returns the colour at the UV coord
    math::Vec3 sampleNormal(float u, float v) const; // returns the converted colour to normal at the UV coord
    Greyscale sampleSpecular(float u, float v) const; // returns the specular intensity at the UV coord
};

const Colour COLOUR_BLACK = Colour(0, 0, 0, 255);
const Colour COLOUR_WHITE = Colour(255, 255, 255, 255);
const Colour COLOUR_NORMAL = Colour(127, 127, 255, 255); // default normal map colour for a flat surface
const Colour COLOUR_MAGENTA = Colour(255, 0, 255, 255); // default colour for missing textures

const Colour COLOUR_RED = Colour(255, 0, 0, 255);
const Colour COLOUR_GREEN = Colour(0, 255, 0, 255);
const Colour COLOUR_BLUE = Colour(0, 0, 255, 255);

const Colour COLOUR_SKY = Colour(99, 151, 235, 255);


};

#endif