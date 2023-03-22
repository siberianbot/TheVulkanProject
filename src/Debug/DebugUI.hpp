#ifndef DEBUG_DEBUGUI_HPP
#define DEBUG_DEBUGUI_HPP

#include <map>
#include <memory>
#include <string>
#include <vector>

#include "src/Types/LightSourceType.hpp"

class EngineVars;
class EventQueue;
class ResourceManager;
class SceneManager;
class Window;
class SceneNode;
class MeshResource;
class ImageResource;
class ShaderResource;

static constexpr const char *NONE_ITEM = "none";

class DebugUI {
private:
    std::shared_ptr<EngineVars> _engineVars;
    std::shared_ptr<EventQueue> _eventQueue;
    std::shared_ptr<ResourceManager> _resourceManager;
    std::shared_ptr<SceneManager> _sceneManager;
    std::shared_ptr<Window> _window;

    std::map<LightSourceType, std::string> _lightTypes = {
            {POINT_LIGHT_SOURCE, toString(POINT_LIGHT_SOURCE)},
            {CONE_LIGHT_SOURCE, toString(CONE_LIGHT_SOURCE)},
            {RECTANGLE_LIGHT_SOURCE, toString(RECTANGLE_LIGHT_SOURCE)}
    };

    std::shared_ptr<ShaderResource> _selectedShaderResource = nullptr;
    std::string _selectedShaderName = NONE_ITEM;
    std::string _selectedShaderCode;
    std::string _selectedShaderStatus;

    std::shared_ptr<SceneNode> _selectedSceneNode = nullptr;
//    std::string _selectedObjectMeshName = NONE_ITEM;
//    std::string _selectedObjectAlbedoTextureName = NONE_ITEM;
//    std::string _selectedObjectSpecularTextureName = NONE_ITEM;

    float _floatStep = 0.1;
    float _floatFastStep = 0.5;
    int _engineFpsWindowVisible = 0;
    int _engineVarsWindowVisible = 0;
    int _rendererShaderEditorVisible = 0;
    int _sceneObjectsWindowVisible = 0;
    int _resourcesListWindowVisible = 0;

    void drawMainMenu();
    void drawEngineFpsWindow();
    void drawEngineVarsWindow();
    void drawRendererShaderEditor();
    void drawSceneObjectsWindow();
    void drawResourcesListWindow();

public:
    DebugUI(const std::shared_ptr<EngineVars> &engineVars,
            const std::shared_ptr<EventQueue> &eventQueue,
            const std::shared_ptr<ResourceManager> &resourceManager,
            const std::shared_ptr<SceneManager> &sceneManager,
            const std::shared_ptr<Window> &window);

    void render();
};

#endif // DEBUG_DEBUGUI_HPP
