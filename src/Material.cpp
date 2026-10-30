#include "Material.hpp"

#include <cmath>
#include <utility>

constexpr float INV_255 = 1.0f / 255.0f;
constexpr float INV_128 = 1.0f / 128.0f;

namespace hybriddisplay::graphics {

float wrap(float uv) {
    return uv - std::floor(uv);
}

float bound(float uv) {
    return std::clamp(uv, 0.0f, 1.0f);
}

Greyscale colourToGreyscale(const Colour& colour) {
    return static_cast<Greyscale>(0.299f * colour.r + 0.587f * colour.g + 0.114f * colour.b);
}


template<typename T>
const T& Image<T>::get(uint32_t x, uint32_t y) const {
    return data[y * size.width + x];
}

template<typename T>
const T& Image<T>::sample(float u, float v) const {
    u -= std::floor(u);
    v = 1.0f - (v - std::floor(v));

    return get(
        static_cast<uint32_t>(u * (size.width - 1)),
        static_cast<uint32_t>(v * (size.height - 1))
    );
}

Colour::Colour(uint8_t r, uint8_t g, uint8_t b, uint8_t a) {
    this->r = r;
    this->g = g;
    this->b = b;
    this->a = a;
}

Colour::Colour(uint32_t AARRGGBB) {
    a = (AARRGGBB >> 24) & 0xFF;
    r = (AARRGGBB >> 16) & 0xFF;
    g = (AARRGGBB >> 8)  & 0xFF;
    b = AARRGGBB & 0xFF;
}

Colour::Colour(math::Vec3 vec3) {
    const auto toChannel = [](float component) {
        const float mapped = (std::clamp(component, -1.0f, 1.0f) + 1.0f) * 127.5f;
        return static_cast<uint8_t>(std::lround(mapped));
    };

    r = toChannel(vec3.x);
    g = toChannel(vec3.y);
    b = toChannel(vec3.z);
    a = 255;
}

Colour Colour::layer(const Colour& top) {
    float alpha = top.a * INV_255;

    return Colour(
        top.r * alpha + r * (1.0f - alpha),
        top.g * alpha + g * (1.0f - alpha),
        top.b * alpha + b * (1.0f - alpha),
        255
    );
}

Colour::operator uint32_t() const {
    return (static_cast<uint32_t>(a) << 24) |
        (static_cast<uint32_t>(r) << 16) |
        (static_cast<uint32_t>(g) << 8)  |
        static_cast<uint32_t>(b);
}

Colour::operator math::Vec3() const {
    return math::Vec3(r * INV_128 - 1.0f, g * INV_128 - 1.0f, b * INV_128 - 1.0f).normalize();
}


Material::Material() {
    diffuse = COLOUR_BLACK;
    specular = COLOUR_BLACK;
    ambient = COLOUR_BLACK;
    reflectiveness = 0;

    std::shared_ptr<Image<Colour>> defaultTexture = std::make_shared<Image<Colour>>(1, 1);
    defaultTexture->data.at(0) = COLOUR_MAGENTA;
    textureMap = defaultTexture;

    std::shared_ptr<Image<math::Vec3>> defaultNormal = std::make_shared<Image<math::Vec3>>(1, 1);
    defaultNormal->data.at(0) = static_cast<math::Vec3>(COLOUR_NORMAL);
    normalMap = defaultNormal;

    specularMap = std::make_shared<Image<Greyscale>>(0, 0);
}

Material::Material(const Colour& _diffuse, const Colour& _specular, const Colour& _ambient, float _reflectiveness, std::shared_ptr<const Image<Colour>> image, std::shared_ptr<const Image<math::Vec3>> normal) {
    diffuse = _diffuse;
    specular = _specular;
    ambient = _ambient;
    reflectiveness = _reflectiveness;
    
    loadTextureMap(image);
    loadNormalMap(normal);
}

void Material::loadTextureMap(std::shared_ptr<const Image<Colour>> image) {
    textureMap = std::move(image);
}

void Material::loadNormalMap(std::shared_ptr<const Image<math::Vec3>> image) {
    normalMap = std::move(image);
}

void Material::loadSpecularMap(std::shared_ptr<const Image<Greyscale>> image) {
    specularMap = std::move(image);
}


Colour Material::sampleTexture(float u, float v) const {
    if (!textureMap || textureMap->data.empty())
        return COLOUR_MAGENTA;

    return textureMap->sample(u,v);
}

math::Vec3 Material::sampleNormal(float u, float v) const {
    if (!normalMap || normalMap->data.empty())
        return math::Vec3(0, 0, 1); // default normal pointing out of the surface

    return normalMap->sample(u,v);
}

Greyscale Material::sampleSpecular(float u, float v) const {
    if (!specularMap || specularMap->data.empty())
        return 0; // default specular intensity

    return specularMap->sample(u,v);
}


};