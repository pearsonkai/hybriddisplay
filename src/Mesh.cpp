#include "Mesh.hpp"
#include <fstream>
#include <sstream>
#include <stdexcept>
#include <unordered_map>
#include <cstdlib>
#include <iostream>
#include <cmath>
#include <functional>

namespace hybriddisplay::geometry {

Mesh::Mesh() {

}

Mesh::Mesh(fs::path obj, bool interpolateNormals) {
    std::ifstream objFile(obj);
    if (!objFile.is_open()) {
        throw std::runtime_error("Failed to open OBJ file: " + obj.string());
    }

    std::vector<math::Vec3> positions;
    std::vector<math::Vec3> normals;
    std::vector<math::Vec3> uvs;
    std::vector<fs::path> materialLibraries;
    std::vector<std::string> faceMaterialNames;
    std::string currentMaterialName;
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
        } else if (prefix == "mtllib") {
            std::string libraryName;
            while (iss >> libraryName) {
                materialLibraries.push_back(obj.parent_path() / libraryName);
            }
        } else if (prefix == "usemtl") {
            iss >> currentMaterialName;
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

            if (interpolateNormals) {
                for (size_t faceIndex = 1; faceIndex + 1 < face.size(); ++faceIndex) {
                    const IVertex triangle[] = {
                        face[0], face[faceIndex], face[faceIndex + 1]
                    };

                    for (const IVertex& vertex : triangle) {
                        vertexIndices.push_back(appendVertex(vertex));
                    }
                    faceMaterialNames.push_back(currentMaterialName);
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
                    faceMaterialNames.push_back(currentMaterialName);
                }
            }
        }
    }

    objFile.close();

    std::unordered_map<std::string, uint32_t> materialLookup;
    loadMaterials(materialLibraries, materialLookup);
    materialIndices.reserve(faceMaterialNames.size());
    for (const std::string& materialName : faceMaterialNames) {
        const auto material = materialLookup.find(materialName);
        materialIndices.push_back(material == materialLookup.end() ? 0 : material->second);
    }

    if (interpolateNormals) {
        *this = this->interpolateNormals();
    }
}



void Mesh::loadMaterials(const std::vector<fs::path>& materialLibraries, std::unordered_map<std::string, uint32_t>& materialLookup) {
    auto fallback = std::make_shared<graphics::Material>();
    materials.push_back(fallback.get());
    ownedMaterials.push_back(std::move(fallback));
    materialLookup.emplace("", 0);

    for (const fs::path& libraryPath : materialLibraries) {
        std::ifstream materialFile(libraryPath);
        if (!materialFile.is_open()) {
            throw std::runtime_error("Failed to open material library: " + libraryPath.string());
        }

        std::shared_ptr<graphics::Material> currentMaterial;
        std::string line;
        while (std::getline(materialFile, line)) {
            std::istringstream iss(line);
            std::string directive;
            iss >> directive;

            if (directive == "newmtl") {
                std::string name;
                std::getline(iss >> std::ws, name);
                currentMaterial = std::make_shared<graphics::Material>();
                materialLookup[name] = static_cast<uint32_t>(materials.size());
                materials.push_back(currentMaterial.get());
                ownedMaterials.push_back(currentMaterial);
            } else if (directive == "map_Kd" && currentMaterial) {
                std::string textureName;
                std::getline(iss >> std::ws, textureName);
                const fs::path texturePath = libraryPath.parent_path() / textureName;
                if (!fs::exists(texturePath)) {
                    throw std::runtime_error("Failed to find material texture: " + texturePath.string());
                }
                currentMaterial->loadTextureMap(graphics::Material::loadImage(texturePath));
            } else if (currentMaterial && (directive == "map_bump" || directive == "bump")) {
                std::vector<std::string> tokens;
                std::string token;
                while (iss >> token) tokens.push_back(token);

                size_t pathStart = 0;
                for (size_t i = 0; i < tokens.size() && !tokens[i].empty() && tokens[i][0] == '-';) {
                    const std::string option = tokens[i++];
                    size_t values = 0;
                    if (option == "-bm" || option == "-boost" || option == "-clamp" ||
                        option == "-imfchan" || option == "-type" || option == "-texres" ||
                        option == "-blendu" || option == "-blendv" || option == "-cc" ||
                        option == "-colorspace") values = 1;
                    else if (option == "-mm") values = 2;
                    else if (option == "-o" || option == "-s" || option == "-t") {
                        while (i < tokens.size() && values < 3) {
                            char* end = nullptr;
                            std::strtof(tokens[i].c_str(), &end);
                            if (end == tokens[i].c_str() || *end != '\0') break;
                            ++i;
                            ++values;
                        }
                    } else {
                        throw std::runtime_error("Unsupported bump map option: " + option);
                    }

                    if (values && option != "-o" && option != "-s" && option != "-t") {
                        if (i + values > tokens.size())
                            throw std::runtime_error("Missing value for bump map option: " + option);
                        i += values;
                    }
                    pathStart = i;
                }

                std::string normalName;
                for (size_t i = pathStart; i < tokens.size(); ++i) {
                    if (!normalName.empty()) normalName += ' ';
                    normalName += tokens[i];
                }
                if (normalName.empty())
                    throw std::runtime_error("Missing bump map texture in: " + libraryPath.string());

                const fs::path texturePath = libraryPath.parent_path() / normalName;
                if (!fs::exists(texturePath)) {
                    throw std::runtime_error("Failed to find material texture: " + texturePath.string());
                }
                currentMaterial->loadNormalMap(graphics::Material::loadImage(texturePath));
            }
        }
    }
}

Mesh Mesh::interpolateNormals() const {
    struct PositionKey {
        float x;
        float y;
        float z;

        bool operator==(const PositionKey& other) const {
            return x == other.x && y == other.y && z == other.z;
        }
    };
    struct PositionHash {
        size_t operator()(const PositionKey& position) const {
            size_t hash = std::hash<float>{}(position.x);
            hash ^= std::hash<float>{}(position.y) + 0x9e3779b9 + (hash << 6) + (hash >> 2);
            hash ^= std::hash<float>{}(position.z) + 0x9e3779b9 + (hash << 6) + (hash >> 2);
            return hash;
        }
    };
    const auto keyFor = [](const math::Vec3& position) {
        return PositionKey{position.x, position.y, position.z};
    };

    std::unordered_map<PositionKey, math::Vec3, PositionHash> normalSums;
    normalSums.reserve(vertices.size());
    for (size_t index = 0; index + 2 < vertexIndices.size(); index += 3) {
        const Vertex& a = vertices[vertexIndices[index]];
        const Vertex& b = vertices[vertexIndices[index + 1]];
        const Vertex& c = vertices[vertexIndices[index + 2]];
        const math::Vec3 faceNormal = (b.position - a.position).cross(c.position - a.position);
        if (!std::isfinite(faceNormal.x) || !std::isfinite(faceNormal.y) ||
            !std::isfinite(faceNormal.z) || faceNormal.magnitude() == 0.0f) {
            continue;
        }

        normalSums[keyFor(a.position)] += faceNormal;
        normalSums[keyFor(b.position)] += faceNormal;
        normalSums[keyFor(c.position)] += faceNormal;
    }

    Mesh result;
    result.vertices = vertices;
    result.vertexIndices = vertexIndices;
    result.materialIndices = materialIndices;

    for (Vertex& vertex : result.vertices) {
        const auto normal = normalSums.find(keyFor(vertex.position));
        if (normal != normalSums.end()) {
            vertex.normal = normal->second.normalize();
        } else if (std::isfinite(vertex.normal.x) && std::isfinite(vertex.normal.y) &&
                   std::isfinite(vertex.normal.z) && vertex.normal.magnitude() > 0.0f) {
            vertex.normal = vertex.normal.normalize();
        } else {
            vertex.normal = math::Vec3(0.0f, 0.0f, 0.0f);
        }
    }

    std::unordered_map<const graphics::Material*, std::shared_ptr<graphics::Material>> materialCopies;
    materialCopies.reserve(ownedMaterials.size() + materials.size());
    result.ownedMaterials.reserve(ownedMaterials.size() + materials.size());
    for (const auto& material : ownedMaterials) {
        if (!material) {
            result.ownedMaterials.push_back(nullptr);
            continue;
        }

        auto copy = std::make_shared<graphics::Material>(*material);
        materialCopies.emplace(material.get(), copy);
        result.ownedMaterials.push_back(std::move(copy));
    }

    result.materials.reserve(materials.size());
    for (const graphics::Material* material : materials) {
        if (!material) {
            result.materials.push_back(nullptr);
            continue;
        }

        auto copy = materialCopies.find(material);
        if (copy == materialCopies.end()) {
            auto ownedCopy = std::make_shared<graphics::Material>(*material);
            copy = materialCopies.emplace(material, std::move(ownedCopy)).first;
            result.ownedMaterials.push_back(copy->second);
        }
        result.materials.push_back(copy->second.get());
    }

    return result;
}












uint32_t Mesh::getNumFaces() const {
    return static_cast<uint32_t>(vertexIndices.size() / 3);
}

uint32_t Mesh::getNumVertices() const {
    return static_cast<uint32_t>(vertices.size());
}

Triangle Mesh::getTri(uint32_t index) const {
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

uint32_t Mesh::getIndice(uint32_t index) const {
    return vertexIndices[index];
}

std::vector<uint32_t> Mesh::getTriIndices(uint32_t index) const {
    const uint32_t faceIndex = index * 3;
    return {
        vertexIndices[faceIndex],
        vertexIndices[faceIndex + 1],
        vertexIndices[faceIndex + 2]
    };
}

std::vector<Triangle> Mesh::getAllTri() const {
    std::vector<Triangle> triangles;
    triangles.reserve(getNumFaces());

    for (uint32_t i = 0; i < getNumFaces(); ++i) {
        triangles.push_back(getTri(i));
    }

    return triangles;
}

const Vertex& Mesh::getVertex(uint32_t index) const {
    return vertices[index];
}

std::vector<Vertex> Mesh::getAllVertices() const {
    return vertices;
}


graphics::Material* Mesh::getMaterial(uint32_t index) const {
    return materials[materialIndices[index]];
}

}