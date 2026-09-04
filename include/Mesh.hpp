#ifndef MESH_HPP
#define MESH_HPP

#include "Material.hpp"

namespace hybriddisplay::geometry {
    
struct Vertex {
    math::Vec3 position;
    math::Vec3 normal;
    math::Vec3 uv;
    math::Vec3 tangent, bitangent; // tangent and bitangent vectors for normal mapping
};

struct Triangle {
    const Vertex *v0, *v1, *v2;
    const graphics::Material* material;
};

class Mesh {
private:
    std::vector<uint32_t> vertexIndices;
    std::vector<uint32_t> materialIndices;
    
    std::vector<Vertex> vertices;
    std::vector<graphics::Material*> materials;
public:
    Mesh();
    Mesh(const std::vector<Vertex>& _vertices, const std::vector<uint32_t>& _vertexIndices, const std::vector<graphics::Material*>& _materials, const std::vector<uint32_t>& _materialIndices);    
    Mesh(fs::path obj);
    
    uint32_t getNumFaces();
    Triangle getTri(uint32_t index);
    std::vector<Triangle> getAllTri();
};

};

#endif