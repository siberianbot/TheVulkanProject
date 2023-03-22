#include "World.hpp"

#include <sstream>

#include "src/Objects/Components/SkyboxComponent.hpp"

World::World()
        : _skybox(std::make_shared<SkyboxComponent>()) {
    this->_components.push_back(this->_skybox);
}

std::string World::displayName() {
    std::stringstream ss;
    ss << "(" << this->id() << ") world";

    return ss.str();
}
