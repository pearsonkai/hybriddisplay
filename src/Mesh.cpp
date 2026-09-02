#include "Mesh.hpp"

namespace hybriddisplay::geometry {

Mesh::Mesh() {

}

Mesh::Mesh(const std::vector<Vertex>& _vertices, const std::vector<uint32_t>& _vertexIndices, const std::vector<graphics::Material*>& _materials, const std::vector<uint32_t>& _materialIndices) {
    vertices = _vertices;
    vertexIndices = _vertexIndices;
    materials = _materials;
    materialIndices = _materialIndices;
}

uint32_t Mesh::getNumFaces() {
    return static_cast<uint32_t>(vertexIndices.size() / 3);
}

Triangle Mesh::getTri(uint32_t index) {
    const uint32_t faceIndex = index * 3;
    const uint32_t v0 = vertexIndices[faceIndex];
    const uint32_t v1 = vertexIndices[faceIndex + 1];
    const uint32_t v2 = vertexIndices[faceIndex + 2];

    const graphics::Material* material = nullptr;
    if (!materials.empty()) {
        if (materialIndices.empty()) {
            material = materials[index % materials.size()];
        } else {
            material = materials[materialIndices[index]];
        }
    }

    return {
        &vertices[v0],
        &vertices[v1],
        &vertices[v2],
        material
    };
}

std::vector<Triangle> Mesh::getAllTri() {
    std::vector<Triangle> triangles;
    triangles.reserve(getNumFaces());

    for (uint32_t i = 0; i < getNumFaces(); ++i) {
        triangles.push_back(getTri(i));
    }

    return triangles;
}

}