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
    std::vector<Model> visible;
public:
    
    World();

    const std::vector<Model>& getVisibleModels() const;
    void addModel(std::unique_ptr<geometry::Mesh> mesh, const math::Transform& transform);
};

};

#endif