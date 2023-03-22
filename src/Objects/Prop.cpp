#include "Prop.hpp"

#include <sstream>

#include "src/Objects/Components/ModelComponent.hpp"
#include "src/Objects/Components/PositionComponent.hpp"

Prop::Prop()
        : _position(std::make_shared<PositionComponent>()),
          _model(std::make_shared<ModelComponent>()) {
    this->_components.push_back(this->_position);
    this->_components.push_back(this->_model);
}

std::string Prop::displayName() {
    std::stringstream ss;
    ss << "(" << this->id() << ") prop";

    return ss.str();
}
