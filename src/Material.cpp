#include "Material.hpp"

#include <algorithm>
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

Colour Colour::blend(const Colour& topLeft, const Colour& topRight, const Colour& bottomLeft, const Colour& bottomRight, float u, float v) {
    const auto value = [u, v](uint8_t a, uint8_t b, uint8_t c, uint8_t d) {
        return static_cast<uint8_t>(std::lround((a + (b - a) * u) * (1.0f - v) + (c + (d - c) * u) * v));
    };

    return Colour(
        value(topLeft.r, topRight.r, bottomLeft.r, bottomRight.r),
        value(topLeft.g, topRight.g, bottomLeft.g, bottomRight.g),
        value(topLeft.b, topRight.b, bottomLeft.b, bottomRight.b),
        value(topLeft.a, topRight.a, bottomLeft.a, bottomRight.a)
    );
}

Colour::operator Greyscale() const {
    return static_cast<Greyscale>(0.299f * r + 0.587f * g + 0.114f * b);
}

Colour::operator uint32_t() const {
    uint32_t AA = static_cast<uint32_t>(a) << 24;
    uint32_t RR = static_cast<uint32_t>(r) << 16;
    uint32_t GG = static_cast<uint32_t>(g) << 8;
    uint32_t BB = static_cast<uint32_t>(b) << 0;
    
    return AA | RR | GG | BB;
}

Colour::operator math::Vec3() const {
    return math::Vec3(r * INV_128 - 1.0f, g * INV_128 - 1.0f, b * INV_128 - 1.0f).normalize();
}


Material::Material() {
    diffuse = COLOUR_BLACK;
    specular = COLOUR_BLACK;
    ambient = COLOUR_BLACK;
    reflectiveness = 0;
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

void Material::setAmbient(const Colour& colour) {
    ambient = colour;
}

void Material::setDiffuse(const Colour& colour) {
    diffuse = colour;
}

void Material::setSpecular(const Colour& colour) {
    specular = colour;
}

float Material::getReflectiveness() const {
    return reflectiveness;
}

void Material::setReflectiveness(float value) {
    reflectiveness = std::clamp(value, 0.0f, 1.0f);
}


Colour Material::sampleTexture(float u, float v) const {
    if (!textureMap || textureMap->data.empty())
        return diffuse;

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