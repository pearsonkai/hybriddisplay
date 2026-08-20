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
};

class World {
private:
    std::vector<Model> visible;
public:
    
    World();

    std::vector<Model>& getVisibleModels();
    void addModel(Mesh* mesh, const math::Transform& transform = math::Transform());
};

};

#endif