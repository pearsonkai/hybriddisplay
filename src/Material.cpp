#include "Material.hpp"

#define STB_IMAGE_IMPLEMENTATION
#include "STB/stb_image.h"
#include <stdexcept>
#include <utility>

namespace hybriddisplay::graphics {

Image<Colour> Material::loadImage(const fs::path& filePath)
{
    int width, height, channels;
    unsigned char* raw = stbi_load(filePath.string().c_str(), &width, &height, &channels, 4);
    if (!raw) {
        throw std::runtime_error("Failed to load image: " + filePath.string());
    }

    const size_t pixelCount = static_cast<size_t>(width) * height;
    std::vector<Colour> pixels(pixelCount);
    for (size_t i = 0; i < pixelCount; ++i) {
        pixels[i] = Colour(raw[i * 4], raw[i * 4 + 1], raw[i * 4 + 2], raw[i * 4 + 3]);
    }
    stbi_image_free(raw);

    Image<Colour> image(static_cast<uint32_t>(width), static_cast<uint32_t>(height));
    image.data = std::move(pixels);
    return image;
}

constexpr float INV_255 = 1.0f / 255.0f;

Colour::Colour(uint8_t r, uint8_t g, uint8_t b, uint8_t a) {
    this->r = r;
    this->g = g;
    this->b = b;
    this->a = a;
}



Material::Material() {
    diffuse = COLOUR_BLACK;
    specular = COLOUR_BLACK;
    ambient = COLOUR_BLACK;
    reflectiveness = 0;

    textureMap = Image<Colour>(1,1);
    textureMap.data.at(0) = COLOUR_MAGENTA;

    normalMap = Image<math::Vec3>(1,1);
    normalMap.data.at(0) = colourToVec3(COLOUR_NORMAL);
}

Material::Material(const Colour& _diffuse, const Colour& _specular, const Colour& _ambient, float _reflectiveness, const Image<Colour>& image, const Image<Colour>& normal) {
    diffuse = _diffuse;
    specular = _specular;
    ambient = _ambient;
    reflectiveness = _reflectiveness;
    
    textureMap.data.resize(image.size.width * image.size.height);
    normalMap.data.resize(normal.size.width * normal.size.height);
    
    loadTextureMap(image);
    loadNormalMap(normal);
}

void Material::loadTextureMap(const Image<Colour>& image)
{
    textureMap = image;
}

void Material::loadNormalMap(const Image<Colour>& image)
{
    normalMap.data.clear();
    normalMap.size = image.size;
    normalMap.data.resize(static_cast<size_t>(image.size.width) * image.size.height);

    for (size_t i = 0; i < image.data.size(); ++i) {
        normalMap.data[i] = colourToVec3(image.data[i]);
    }
}

void Material::loadSpecularMap(const Image<Colour>& image)
{
    specularMap.data.clear();
    specularMap.size = image.size;
    specularMap.data.resize(static_cast<size_t>(image.size.width) * image.size.height);

    for (size_t i = 0; i < image.data.size(); ++i) {
        specularMap.data[i] = colourToGreyscale(image.data[i]);
    }
}

float Material::wrap(float uv) {
    return uv - std::floor(uv);
}

float Material::bound(float uv) {
    return std::clamp(uv, 0.0f, 1.0f);
}

Colour Material::sampleTexture(float u, float v) const {
    if (textureMap.data.empty())
        return COLOUR_MAGENTA;

    u = wrap(u);
    v = 1.0f - wrap(v);

    return textureMap.get(
        static_cast<uint32_t>(u * (textureMap.size.width - 1)),
        static_cast<uint32_t>(v * (textureMap.size.height - 1))
    );
}

math::Vec3 Material::sampleNormal(float u, float v) const {
    if (normalMap.data.empty())
        return math::Vec3(0, 0, 1); // default normal pointing out of the surface

    u = wrap(u);
    v = wrap(v);

    return normalMap.get(
        static_cast<uint32_t>(u * (normalMap.size.width - 1)),
        static_cast<uint32_t>(v * (normalMap.size.height - 1))
    );
}

Greyscale Material::sampleSpecular(float u, float v) const {
    if (specularMap.data.empty())
        return 0; // default specular intensity

    u = wrap(u);
    v = wrap(v);

    return specularMap.get(
        static_cast<uint32_t>(u * (specularMap.size.width - 1)),
        static_cast<uint32_t>(v * (specularMap.size.height - 1))
    );
}

math::Vec3 Material::colourToVec3(const Colour& colour) {
    return math::Vec3(colour.r * INV_255, colour.g * INV_255, colour.b * INV_255).normalize();
}

Greyscale Material::colourToGreyscale(const Colour& colour) {
    return static_cast<Greyscale>(0.299f * colour.r + 0.587f * colour.g + 0.114f * colour.b);
}

};