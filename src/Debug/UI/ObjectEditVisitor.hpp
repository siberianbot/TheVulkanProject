#ifndef DEBUG_UI_OBJECTEDITVISITOR_HPP
#define DEBUG_UI_OBJECTEDITVISITOR_HPP

#include <memory>

class Camera;
class LightSource;
class ModelComponent;
class PositionComponent;
class SkyboxComponent;
class ResourceManager;

class ObjectEditVisitor {
private:
    std::shared_ptr<ResourceManager> _resourceManager;

public:
    ObjectEditVisitor(const std::shared_ptr<ResourceManager> &resourceManager);

    void drawCameraObject(Camera *camera);
    void drawLightSourceObject(LightSource *lightSource);

    void drawModelComponent(ModelComponent *texture);
    void drawPositionComponent(PositionComponent *component);
    void drawSkyboxComponent(SkyboxComponent *component);
};

#endif // DEBUG_UI_OBJECTEDITVISITOR_HPP
