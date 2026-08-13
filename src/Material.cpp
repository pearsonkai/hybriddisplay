#include "Material.hpp"

#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"

namespace hybriddisplay::graphics {

constexpr float INV_255 = 1.0f / 255.0f;

Material::Material(const Colour& diffuse, const Colour& specular, const Colour& ambient = Colour(0, 0, 0, 255), float reflectiveness = 0.5f)
{
    this->diffuse = diffuse;
    this->specular = specular;
    this->ambient = ambient;
    this->reflectiveness = reflectiveness;

    // Initialize texture, normal, and specular maps with default sizes
    textureMap = Image<Colour>(1, 1); // default 1x1 texture map
    normalMap = Image<Vec3>(1, 1); // default 1x1 normal map
    specularMap = Image<Greyscale>(1, 1); // default 1x1 specular map
}

void Material::loadTextureMap(const Image<Colour>& image)
{
    textureMap = image;
}

void Material::loadNormalMap(const Image<Colour>& image)
{
    normalMap.clear();
    normalMap.size = image.size;
    normalMap.resize(image.size.width, image.size.height);

    for (int i = 0; i < image.size.width * image.size.height; ++i) {
        normalMap.data[i] = colourToVec3(image.data[i]);
    }
}

void Material::loadSpecularMap(const Image<Colour>& image)
{
    specularMap.clear();
    specularMap.size = image.size;
    specularMap.resize(image.size.width, image.size.height);

    for (int i = 0; i < image.size.width * image.size.height; ++i) {
        specularMap.data[i] = colourToGreyscale(image.data[i]);
    }
}

float Material::wrap(float uv) const {
    return uv - std::floor(uv);
}

float Material::bound(float uv) const {
    return std::clamp(uv, 0.0f, 1.0f);
}

Colour Material::sampleTexture(float u, float v) const
{
    if (textureMap.data.empty())
        return MAGENTA;

    u = wrap(u);
    v = wrap(v);

    return textureMap(static_cast<uint32_t>(u * (textureMap.size.width - 1)), static_cast<uint32_t>(v * (textureMap.size.height - 1)));
}

Vec3 Material::sampleNormal(float u, float v) const
{
    if (normalMap.data.empty())
        return Vec3(0, 0, 1); // default normal pointing out of the surface

    u = wrap(u);
    v = wrap(v);

    Colour normalColour = normalMap(static_cast<uint32_t>(u * (normalMap.size.width - 1)), static_cast<uint32_t>(v * (normalMap.size.height - 1)));
    return colourToVec3(normalColour);
}

Greyscale Material::sampleSpecular(float u, float v) const
{
    if (specularMap.data.empty())
        return 0; // default specular intensity

    u = wrap(u);
    v = wrap(v);

    return specularMap(static_cast<uint32_t>(u * (specularMap.size.width - 1)), static_cast<uint32_t>(v * (specularMap.size.height - 1)));
}

Image<Colour> Material::loadPNG(const fs::path& filePath) const
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
    return Image<Colour>(width, height, pixels);
}

Vec3 Material::colourToVec3(const Colour& colour) const
{
    return Vec3(colour.r * INV_255, colour.g * INV_255, colour.b * INV_255).normalize();
}

Greyscale Material::colourToGreyscale(const Colour& colour) const
{
    return static_cast<Greyscale>(0.299f * colour.r + 0.587f * colour.g + 0.114f * colour.b);
}

};