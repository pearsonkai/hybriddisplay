#ifndef MESH_HPP
#define MESH_HPP

#include "Material.hpp"
#include <memory>
#include <unordered_map>

namespace hybriddisplay::geometry {
    
struct Vertex {
    math::Vec3 position;
    math::Vec3 normal;
    math::Vec3 uv;
    //math::Vec3 tangent, bitangent; // tangent and bitangent vectors for normal mapping
};

struct Triangle {
    const Vertex *v0, *v1, *v2;
    const graphics::Material* material;
};

class Mesh {
private:
    std::vector<uint32_t> vertexIndices;
    std::vector<Vertex> vertices;

    std::vector<uint32_t> materialIndices;
    std::vector<graphics::Material*> materials;
    std::vector<std::shared_ptr<graphics::Material>> ownedMaterials;

    void loadMaterials(const std::vector<fs::path>& materialLibraries, std::unordered_map<std::string, uint32_t>& materialLookup);
public:
    Mesh();
    Mesh(const std::vector<Vertex>& _vertices, const std::vector<uint32_t>& _vertexIndices, const std::vector<graphics::Material*>& _materials, const std::vector<uint32_t>& _materialIndices);    
    Mesh(fs::path obj, bool duplicateVertices = false);
    Mesh interpolateNormals() const; // return a copied mesh with normals averaged from adjacent faces.
    
    uint32_t getNumFaces() const;
    uint32_t getNumVertices() const;

    uint32_t getIndice(uint32_t) const;
    Triangle getTri(uint32_t index) const;
    std::vector<uint32_t> getTriIndices(uint32_t index) const;
    std::vector<Triangle> getAllTri() const;

    const Vertex& getVertex(uint32_t index) const;
    std::vector<Vertex> getAllVertices() const;

    graphics::Material* getMaterial(uint32_t index) const;
};

};

#endif