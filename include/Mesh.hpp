#ifndef TRANSFORM_HPP
#define TRANSFORM_HPP

#include "Vec3.hpp"

namespace hybriddisplay::geometry {
    
struct Vertex {
    Vec3 position;
    Vec3 normal;
    Vec3 uv;
    Vec3 tangent; // tangent vector for normal mapping
};

struct Triangle {
    Vertex* v1, v2, v3;
};

class Mesh {
private:
    std::vector<Triangle> triangles;
    std::vector<std::unique_ptr<Vertex>> vertices;


public:
    
    
};

};

#endif