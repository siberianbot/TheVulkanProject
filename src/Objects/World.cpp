#include "World.hpp"

#include <fmt/core.h>

#include "src/Objects/Components/SkyboxComponent.hpp"

World::World() : World(std::make_shared<SkyboxComponent>()) {
    //
}

World::World(const std::shared_ptr<SkyboxComponent> &skybox)
        : _skybox(skybox) {
    this->_components.push_back(this->_skybox);
}

std::string World::displayName() {
    return fmt::format("({0}) world", this->id());
}
