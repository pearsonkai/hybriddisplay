#ifndef WORLD_HPP
#define WORLD_HPP

#include <vector>
#include "Transform.hpp"
#include "Mesh.hpp"

namespace hybriddisplay::geometry {

struct Model {
    std::shared_ptr<Mesh> mesh;
    math::Transform transform;

    Model(std::shared_ptr<Mesh> mesh = nullptr, math::Transform transform = math::Transform());
};

class World {
private:
    std::vector<Model> visible;
public:
    
    World();

    const std::vector<Model>& getVisibleModels() const;
    Model& addModel(std::shared_ptr<Mesh> mesh, const math::Transform& transform); // adds a Model to the visible vector
};

};

#endif