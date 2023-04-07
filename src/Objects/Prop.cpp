#include "Prop.hpp"

#include <fmt/core.h>

#include "src/Objects/Components/ModelComponent.hpp"
#include "src/Objects/Components/PositionComponent.hpp"

Prop::Prop() : Prop(std::make_shared<PositionComponent>(),
                    std::make_shared<ModelComponent>()) {
    //
}

Prop::Prop(const std::shared_ptr<PositionComponent> &position,
           const std::shared_ptr<ModelComponent> &model)
        : _position(position),
          _model(model) {
    this->_components.push_back(this->_position);
    this->_components.push_back(this->_model);
}

std::string Prop::displayName() {
    return fmt::format("({0}) prop", this->id());
}
