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

struct TriangleI {
    uint32_t v1, v2, v3; // vertex indices for the triangle
    uint32_t material; // index of the material used for this triangle
};

struct TriangleP {
    const Vertex* v0, *v1, *v2;
    const rendering::Material* material;
};

class Mesh {
private:
    std::vector<Vertex> vertices;
    std::vector<std::unique_ptr<rendering::Material>> materials;
    
    std::vector<TriangleI> triangles;
public:
    Mesh(fs::path objFile);
    TriangleP getTri(TriangleI indexTri) const;
    TriangleP getTri(uint32_t index) const;
};

};

#endif