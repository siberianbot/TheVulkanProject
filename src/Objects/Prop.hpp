#ifndef OBJECTS_PROP_HPP
#define OBJECTS_PROP_HPP

#include "src/Objects/Object.hpp"

class PositionComponent;
class ModelComponent;

class Prop : public Object {
private:
    std::shared_ptr<PositionComponent> _position;
    std::shared_ptr<ModelComponent> _model;

public:
    explicit Prop();
    ~Prop() override = default;

    std::string displayName() override;

    [[nodiscard]] const std::shared_ptr<PositionComponent> position() const { return this->_position; }
    [[nodiscard]] const std::shared_ptr<ModelComponent> model() const { return this->_model; }
};

#endif // OBJECTS_PROP_HPP
