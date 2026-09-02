#ifndef WORLD_HPP
#define WORLD_HPP

#include <vector>
#include <memory>
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
    std::vector<std::unique_ptr<Mesh>> meshes;
    std::vector<Model> visible;
public:
    
    World();

    const std::vector<Model>& getVisibleModels() const;
    
    void addMesh(Mesh& mesh); // creates a unique_ptr<Mesh> and adds it to the meshes vector
    void addModel(Mesh* mesh, const math::Transform& transform); // adds a Model to the visible vector
};

};

#endif