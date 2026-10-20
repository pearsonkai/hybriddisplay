#include "World.hpp"

namespace hybriddisplay::geometry {

Model::Model(Mesh* mesh, math::Transform transform) {
    this->mesh = mesh;
    this->transform = transform;
}

World::World() : visible() {

}

const std::vector<Model>& World::getVisibleModels() const {
    return visible;
}

Model& World::addModel(Mesh* mesh, const math::Transform& transform) {
    visible.emplace_back(mesh, transform);
    return visible.back();
}

};