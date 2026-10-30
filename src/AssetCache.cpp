#include <Mesh.hpp>
#include <Material.hpp>

#define STB_IMAGE_IMPLEMENTATION
#include "STB/stb_image.h"

#include <stdexcept>
#include <unordered_map>

namespace hybriddisplay::geometry {

static std::unordered_map<fs::path, std::weak_ptr<Mesh>> meshCache;

std::shared_ptr<Mesh> Mesh::load(fs::path filePath, bool interpolateNormals) {
    std::weak_ptr<Mesh>& cachedMesh = meshCache[filePath];
    if (std::shared_ptr<Mesh> mesh = cachedMesh.lock()) {
        return mesh;
    }

    std::shared_ptr<Mesh> mesh = std::shared_ptr<Mesh>(new Mesh(filePath, interpolateNormals));
    cachedMesh = mesh;
    return mesh;
}

// debug checker to see if a mesh is loaded into the meshCache without loading it
std::shared_ptr<Mesh> Mesh::check(fs::path filePath) {
    std::unordered_map<fs::path, std::weak_ptr<Mesh>>::iterator cachedMesh = meshCache.find(filePath);
    if (cachedMesh == meshCache.end()) {
        return nullptr;
    }

    return cachedMesh->second.lock();
}

}

namespace hybriddisplay::graphics {


// constructor for colour image
template<>
Image<Colour>::Image(const fs::path& filePath) {
    int width;
    int height;
    int channels;
    unsigned char* raw = stbi_load(filePath.string().c_str(), &width, &height, &channels, 4);
    if (!raw) {
        throw std::runtime_error("Failed to load image: " + filePath.string());
    }

    const size_t pixelCount = static_cast<size_t>(width) * static_cast<size_t>(height);
    size = {static_cast<uint32_t>(width), static_cast<uint32_t>(height)};
    data.resize(pixelCount);

    for (size_t i = 0; i < pixelCount; ++i) {
        data[i] = Colour(raw[i * 4], raw[i * 4 + 1], raw[i * 4 + 2], raw[i * 4 + 3]);
    }

    stbi_image_free(raw);
}


// constructor for vector image
template<>
Image<math::Vec3>::Image(const fs::path& filePath) {
    Image<Colour> colourImage(filePath);
    size = colourImage.size;
    data.reserve(colourImage.data.size());

    for (const Colour& colour : colourImage.data) {
        data.push_back(static_cast<math::Vec3>(colour));
    }
}

// constructor for greyscale image
template<>
Image<Greyscale>::Image(const fs::path& filePath) {
    Image<Colour> colourImage(filePath);
    size = colourImage.size;
    data.reserve(colourImage.data.size());

    for (const Colour& colour : colourImage.data) {
        data.push_back(static_cast<Greyscale>(
            0.299f * colour.r + 0.587f * colour.g + 0.114f * colour.b
        ));
    }
}



template<typename T>
static std::shared_ptr<const Image<T>> loadImage(const fs::path& filePath) {
    static std::unordered_map<fs::path, std::weak_ptr<const Image<T>>> imageCache;

    std::weak_ptr<const Image<T>>& cachedImage = imageCache[filePath];
    if (std::shared_ptr<const Image<T>> image = cachedImage.lock()) {
        return image;
    }

    std::shared_ptr<const Image<T>> image = std::make_shared<Image<T>>(filePath);
    cachedImage = image;
    return image;
}







// specific Image<T>::load(filePath); declarations for the most common image types

template<>
std::shared_ptr<const Image<Colour>> Image<Colour>::load(const fs::path& filePath) {
    return loadImage<Colour>(filePath);
}

template<>
std::shared_ptr<const Image<math::Vec3>> Image<math::Vec3>::load(const fs::path& filePath) {
    return loadImage<math::Vec3>(filePath);
}

template<>
std::shared_ptr<const Image<Greyscale>> Image<Greyscale>::load(const fs::path& filePath) {
    return loadImage<Greyscale>(filePath);
}

}
