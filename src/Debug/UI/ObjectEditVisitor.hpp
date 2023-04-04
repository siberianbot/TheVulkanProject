#ifndef DEBUG_UI_OBJECTEDITVISITOR_HPP
#define DEBUG_UI_OBJECTEDITVISITOR_HPP

#include <memory>

class ResourceDatabase;

class Camera;
class LightSource;
class ModelComponent;
class PositionComponent;
class SkyboxComponent;

class ObjectEditVisitor {
private:
    std::shared_ptr<ResourceDatabase> _resourceDatabase;

public:
    ObjectEditVisitor(const std::shared_ptr<ResourceDatabase> &resourceDatabase);

    void drawCameraObject(Camera *camera);
    void drawLightSourceObject(LightSource *lightSource);

    void drawModelComponent(ModelComponent *texture);
    void drawPositionComponent(PositionComponent *component);
    void drawSkyboxComponent(SkyboxComponent *component);
};

#endif // DEBUG_UI_OBJECTEDITVISITOR_HPP
