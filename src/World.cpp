#include "World.hpp"

namespace hybriddisplay::geometry {

const std::vector<Model>& World::getVisibleModels() const {
    return visible;
}

void World::addModel(std::unique_ptr<geometry::Mesh> mesh, const math::Transform& transform) {
    visible.emplace_back(std::move(mesh), transform);
}

};