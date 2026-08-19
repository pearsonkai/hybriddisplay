#include "Material.hpp"

#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"

namespace hybriddisplay::graphics {

constexpr float INV_255 = 1.0f / 255.0f;

Material::Material(const Colour& _diffuse, const Colour& _specular, const Colour& _ambient, float _reflectiveness, const Image<Colour>& image, const Image<Colour>& normal)
{
    diffuse = _diffuse;
    specular = _specular;
    ambient = _ambient;
    reflectiveness = _reflectiveness;
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

Colour Material::sampleTexture(float u, float v) const
{
    if (textureMap.data.empty())
        return MAGENTA;

    u = wrap(u);
    v = wrap(v);

    return textureMap.get(
        static_cast<uint32_t>(u * (textureMap.size.width - 1)),
        static_cast<uint32_t>(v * (textureMap.size.height - 1))
    );
}

math::Vec3 Material::sampleNormal(float u, float v) const
{
    if (normalMap.data.empty())
        return math::Vec3(0, 0, 1); // default normal pointing out of the surface

    u = wrap(u);
    v = wrap(v);

    return normalMap.get(
        static_cast<uint32_t>(u * (normalMap.size.width - 1)),
        static_cast<uint32_t>(v * (normalMap.size.height - 1))
    );
}

Greyscale Material::sampleSpecular(float u, float v) const
{
    if (specularMap.data.empty())
        return 0; // default specular intensity

    u = wrap(u);
    v = wrap(v);

    return specularMap.get(
        static_cast<uint32_t>(u * (specularMap.size.width - 1)),
        static_cast<uint32_t>(v * (specularMap.size.height - 1))
    );
}

Image<Colour> Material::loadPNG(const fs::path& filePath)
{
    int width, height, channels;
    unsigned char* raw = stbi_load(filePath.string().c_str(), &width, &height, &channels, 4);

    std::vector<Colour> pixels(width * height);

    for (int i = 0; i < width * height; ++i)
    {
        pixels[i] = Colour(
            raw[i * 4 + 0], // R
            raw[i * 4 + 1], // G
            raw[i * 4 + 2], // B
            raw[i * 4 + 3]  // A
        );
    }
    stbi_image_free(raw);

    Image<Colour> image = Image<Colour>(width, height);
    image.data = pixels;
    return image;
}

math::Vec3 Material::colourToVec3(const Colour& colour)
{
    return math::Vec3(colour.r * INV_255, colour.g * INV_255, colour.b * INV_255).normalize();
}

Greyscale Material::colourToGreyscale(const Colour& colour)
{
    return static_cast<Greyscale>(0.299f * colour.r + 0.587f * colour.g + 0.114f * colour.b);
}

};