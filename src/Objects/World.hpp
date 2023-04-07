#ifndef OBJECTS_WORLD_HPP
#define OBJECTS_WORLD_HPP

#include "src/Objects/Object.hpp"

class SkyboxComponent;

class World : public Object {
private:
    std::shared_ptr<SkyboxComponent> _skybox;

public:
    explicit World();
    explicit World(const std::shared_ptr<SkyboxComponent> &skybox);

    ~World() override = default;

    std::string displayName() override;

    [[nodiscard]] const std::shared_ptr<SkyboxComponent> &skybox() const { return this->_skybox; }
};

#endif // OBJECTS_WORLD_HPP
