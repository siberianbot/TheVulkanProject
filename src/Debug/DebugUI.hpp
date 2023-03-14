#ifndef DEBUG_DEBUGUI_HPP
#define DEBUG_DEBUGUI_HPP

#include <optional>
#include <string>
#include <vector>

class Engine;
class Light;
class Object;
class MeshResource;
class ImageResource;
class ShaderResource;

static constexpr const char *NONE_ITEM = "none";

class DebugUI {
private:
    Engine *_engine;

    const char *_lightKinds[3] = {
            "Point",
            "Spot",
            "Rectangle"
    };

    ShaderResource *_selectedShaderResource = nullptr;
    std::string _selectedShaderName = NONE_ITEM;
    std::string _selectedShaderCode;
    std::string _selectedShaderStatus;

    Object *_selectedObject = nullptr;
    std::string _selectedObjectMeshName = NONE_ITEM;
    std::string _selectedObjectAlbedoTextureName = NONE_ITEM;
    std::string _selectedObjectSpecularTextureName = NONE_ITEM;

    std::optional<std::vector<Light *>::const_iterator> _selectedLight = std::nullopt;
    int _selectedLightKindIdx = -1;

    float _floatStep = 0.1;
    float _floatFastStep = 0.5;
    int _engineFpsWindowVisible = 0;
    int _engineVarsWindowVisible = 0;
    int _rendererShaderEditorVisible = 0;
    int _sceneObjectsWindowVisible = 0;
    int _sceneLightsWindowVisible = 0;
    int _resourcesListWindowVisible = 0;

    void drawMainMenu();
    void drawEngineFpsWindow();
    void drawEngineVarsWindow();
    void drawRendererShaderEditor();
    void drawSceneObjectsWindow();
    void drawSceneLightsWindow();
    void drawResourcesListWindow();

public:
    DebugUI(Engine *engine);

    void render();
};

#endif // DEBUG_DEBUGUI_HPP
