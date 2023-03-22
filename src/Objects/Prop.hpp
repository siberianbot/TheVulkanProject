#ifndef OBJECTS_PROP_HPP
#define OBJECTS_PROP_HPP

#include "src/Objects/Object.hpp"

struct PositionComponent;
struct ModelComponent;

class Prop : public Object {
private:
    std::shared_ptr<PositionComponent> _position;
    std::shared_ptr<ModelComponent> _model;

public:
    explicit Prop();
    ~Prop() override = default;

    std::string displayName() override;
};

#endif // OBJECTS_PROP_HPP
