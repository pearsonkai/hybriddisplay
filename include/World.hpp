#ifndef WORLD_HPP
#define WORLD_HPP

#include <vector>
#include "Transform.hpp"
#include "Mesh.hpp"

namespace hybriddisplay::geometry {

struct Model {
    Mesh* mesh;
    math::Transform transform;

    Model(Mesh* mesh = nullptr, math::Transform transform = math::Transform());
};

class World {
private:
    std::vector<Mesh*> meshes;
    std::vector<Model> visible;
public:
    
    World();

    const std::vector<Model>& getVisibleModels() const;
    
    void addMesh(Mesh& mesh); // registers an externally owned mesh
    Model& addModel(Mesh* mesh, const math::Transform& transform); // adds a Model to the visible vector
};

};

#endif