#include "Mesh.hpp"
#include <fstream>
#include <sstream>
#include <stdexcept>

namespace hybriddisplay::geometry {

Mesh::Mesh() {

}

Mesh::Mesh(const std::vector<Vertex>& _vertices, const std::vector<uint32_t>& _vertexIndices, const std::vector<graphics::Material*>& _materials, const std::vector<uint32_t>& _materialIndices) {
    vertices = _vertices;
    vertexIndices = _vertexIndices;
    materials = _materials;
    materialIndices = _materialIndices;
}

Mesh::Mesh(fs::path obj) {
    
    // Load the OBJ file
    std::ifstream objFile(obj);
    if (!objFile.is_open()) {
        throw std::runtime_error("Failed to open OBJ file: " + obj.string());
    }

    std::vector<math::Vec3> positions;
    std::vector<math::Vec3> normals;
    std::vector<math::Vec3> uvs;

    struct FaceReference {
        int position;
        int uv;
        int normal;
    };

    auto resolveIndex = [](int index, size_t count) -> size_t {
        if (index > 0) {
            const size_t resolved = static_cast<size_t>(index - 1);
            if (resolved < count) {
                return resolved;
            }
        } else if (index < 0) {
            const int resolved = static_cast<int>(count) + index;
            if (resolved >= 0 && static_cast<size_t>(resolved) < count) {
                return static_cast<size_t>(resolved);
            }
        }

        throw std::runtime_error("OBJ index is outside the available data");
    };

    std::string line;
    while (std::getline(objFile, line)) {
        std::istringstream iss(line);
        std::string prefix;
        iss >> prefix;

        if (prefix == "v") {
            float x, y, z;
            iss >> x >> y >> z;
            positions.emplace_back(x, y, z);
        } else if (prefix == "vn") {
            float x, y, z;
            iss >> x >> y >> z;
            normals.emplace_back(x, y, z);
        } else if (prefix == "vt") {
            float u, v;
            iss >> u >> v;
            uvs.emplace_back(u, v, 0.0f); // Store UVs as Vec3 with z=0
        } else if (prefix == "f") {
            std::vector<FaceReference> face;
            std::string faceToken;
            while (iss >> faceToken) {
                FaceReference reference{0, 0, 0};
                std::stringstream tokenStream(faceToken);
                std::string indexPart;
                std::vector<std::string> indexParts;

                while (std::getline(tokenStream, indexPart, '/')) {
                    indexParts.push_back(indexPart);
                }

                if (indexParts.empty() || indexParts.size() > 3 || indexParts[0].empty()) {
                    throw std::runtime_error("Invalid OBJ face reference: " + faceToken);
                }

                reference.position = std::stoi(indexParts[0]);
                if (indexParts.size() > 1 && !indexParts[1].empty()) {
                    reference.uv = std::stoi(indexParts[1]);
                }
                if (indexParts.size() > 2 && !indexParts[2].empty()) {
                    reference.normal = std::stoi(indexParts[2]);
                }
                face.push_back(reference);
            }

            if (face.size() < 3) {
                throw std::runtime_error("OBJ face has fewer than three vertices");
            }

            for (size_t faceIndex = 1; faceIndex + 1 < face.size(); ++faceIndex) {
                const FaceReference triangleReferences[] = {
                    face[0], face[faceIndex], face[faceIndex + 1]
                };

                for (const FaceReference& reference : triangleReferences) {
                Vertex vertex;
                vertex.position = positions[resolveIndex(reference.position, positions.size())];
                vertex.normal = reference.normal == 0 ? math::Vec3(0, 0, 0) : normals[resolveIndex(reference.normal, normals.size())];
                vertex.uv = reference.uv == 0 ? math::Vec3(0, 0, 0) : uvs[resolveIndex(reference.uv, uvs.size())];
                vertices.push_back(vertex);
                vertexIndices.push_back(static_cast<uint32_t>(vertices.size() - 1));
                }
            }
        }
    }

    objFile.close();
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