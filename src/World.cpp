#include "World.hpp"

namespace hybriddisplay::collections {

std::vector<Model>& World::getVisibleModels() {
    return visible;
}

void World::addModel(std::unique_ptr<geometry::Mesh> mesh, const math::Transform& transform) {
    visible.push_back({std::move(mesh), transform});
}

};