#ifndef WORLD_HPP
#define WORLD_HPP

#include <vector>
#include <memory>
#include "Transform.hpp"
#include "Mesh.hpp"

namespace hybriddisplay::collection {

struct Model {
    std::unique_ptr<geometry::Mesh> mesh;
    math::Transform transform;
};

class World {
private:
    std::vector<Model> visible;
public:
    
    World();

    std::vector<Model>& getVisibleModels();
    void addModel(std::unique_ptr<geometry::Mesh> mesh, const math::Transform& transform = math::Transform());
};

};

#endif