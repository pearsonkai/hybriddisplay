#ifndef TRANSFORM_HPP
#define TRANSFORM_HPP

#include "Material.hpp"

namespace hybriddisplay::geometry {
    
struct Vertex {
    Vec3 position;
    Vec3 normal;
    Vec3 uv;
    Vec3 tangent, bitangent; // tangent and bitangent vectors for normal mapping
};

struct TriangleI {
    uint32_t v1, v2, v3; // vertex indices for the triangle
    uint32_t material; // index of the material used for this triangle
};

struct TriangleP {
    const Vertex* v0, *v1, *v2;
    const Material* material;
};

class Mesh {
private:
    std::vector<Vertex> vertices;
    std::vector<std::unique_ptr<Material>> materials;
    
    std::vector<TriangleI> triangles;
public:
    TriangleP getTri(TriangleI indexTri) const;

    TriangleI getTriIndex(uint32_t index) const;
};

};

#endif