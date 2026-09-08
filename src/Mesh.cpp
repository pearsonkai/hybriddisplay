#include "Mesh.hpp"
#include <fstream>
#include <sstream>
#include <stdexcept>
#include <unordered_map>

namespace hybriddisplay::geometry {

Mesh::Mesh() {

}

Mesh::Mesh(const std::vector<Vertex>& _vertices, const std::vector<uint32_t>& _vertexIndices, const std::vector<graphics::Material*>& _materials, const std::vector<uint32_t>& _materialIndices) {
    vertices = _vertices;
    vertexIndices = _vertexIndices;
    materials = _materials;
    materialIndices = _materialIndices;
}

Mesh::Mesh(fs::path obj, bool duplicateVertices) {
    
    // Load the OBJ file
    std::ifstream objFile(obj);
    if (!objFile.is_open()) {
        throw std::runtime_error("Failed to open OBJ file: " + obj.string());
    }

    std::vector<math::Vec3> positions;
    std::vector<math::Vec3> normals;
    std::vector<math::Vec3> uvs;
    std::unordered_map<std::string, uint32_t> vertexMap; // Map to store unique vertex combinations

    struct IVertex {
        uint32_t position;
        uint32_t uv;
        uint32_t normal;
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

    auto appendVertex = [&](const IVertex& vertex) -> uint32_t {
        Vertex meshVertex;
        meshVertex.position = positions[resolveIndex(vertex.position, positions.size())];
        meshVertex.normal = vertex.normal == 0 ? math::Vec3(0, 0, 0) : normals[resolveIndex(vertex.normal, normals.size())];
        meshVertex.uv = vertex.uv == 0 ? math::Vec3(0, 0, 0) : uvs[resolveIndex(vertex.uv, uvs.size())];

        const uint32_t vertexIndex = static_cast<uint32_t>(vertices.size());
        vertices.push_back(meshVertex);
        return vertexIndex;
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

            std::vector<IVertex> face;
            std::vector<std::string> faceTokens;
            std::string faceToken;
            while (iss >> faceToken) {
                IVertex vertex{0, 0, 0};
                std::stringstream tokenStream(faceToken);
                std::string indexPart;
                std::vector<std::string> indexParts;

                while (std::getline(tokenStream, indexPart, '/')) {
                    indexParts.push_back(indexPart);
                }

                if (indexParts.empty() || indexParts.size() > 3 || indexParts[0].empty()) {
                    throw std::runtime_error("Invalid OBJ face reference: " + faceToken);
                }

                vertex.position = std::stoi(indexParts[0]);
                if (indexParts.size() > 1 && !indexParts[1].empty()) {
                    vertex.uv = std::stoi(indexParts[1]);
                }
                if (indexParts.size() > 2 && !indexParts[2].empty()) {
                    vertex.normal = std::stoi(indexParts[2]);
                }

                face.push_back(vertex);
                faceTokens.push_back(faceToken);
            }

            if (face.size() < 3) {
                throw std::runtime_error("OBJ face has fewer than three vertices");
            }

            if (duplicateVertices) {
                for (size_t faceIndex = 1; faceIndex + 1 < face.size(); ++faceIndex) {
                    const IVertex triangle[] = {
                        face[0], face[faceIndex], face[faceIndex + 1]
                    };

                    for (const IVertex& vertex : triangle) {
                        vertexIndices.push_back(appendVertex(vertex));
                    }
                }
            } else {
                std::vector<uint32_t> faceIndices;
                faceIndices.reserve(face.size());

                for (size_t faceIndex = 0; faceIndex < face.size(); ++faceIndex) {
                    const auto existingVertex = vertexMap.find(faceTokens[faceIndex]);
                    if (existingVertex != vertexMap.end()) {
                        faceIndices.push_back(existingVertex->second);
                        continue;
                    }

                    const uint32_t vertexIndex = appendVertex(face[faceIndex]);
                    vertexMap.emplace(faceTokens[faceIndex], vertexIndex);
                    faceIndices.push_back(vertexIndex);
                }

                for (size_t faceIndex = 1; faceIndex + 1 < faceIndices.size(); ++faceIndex) {
                    vertexIndices.push_back(faceIndices[0]);
                    vertexIndices.push_back(faceIndices[faceIndex]);
                    vertexIndices.push_back(faceIndices[faceIndex + 1]);
                }
            }
        }
    }

    objFile.close();
}

uint32_t Mesh::getNumFaces() {
    return static_cast<uint32_t>(vertexIndices.size() / 3);
}

uint32_t Mesh::getNumVertices() {
    return static_cast<uint32_t>(vertices.size());
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

Vertex Mesh::getVertex(uint32_t index) {
    return vertices[index];
}

std::vector<Vertex> Mesh::getAllVertices() {
    return vertices;
}

}