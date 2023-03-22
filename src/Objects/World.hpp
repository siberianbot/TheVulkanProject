#ifndef OBJECTS_WORLD_HPP
#define OBJECTS_WORLD_HPP

#include "src/Objects/Object.hpp"

struct SkyboxComponent;

class World : public Object {
private:
    std::shared_ptr<SkyboxComponent> _skybox;

public:
    explicit World();
    ~World() override = default;

    std::string displayName() override;
};

#endif // OBJECTS_WORLD_HPP
