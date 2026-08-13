#include "Material.h"

namespace hybriddisplay::graphics {

float Material::wrap(float uv) const {
    return shininess;
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

};