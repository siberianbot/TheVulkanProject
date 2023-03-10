#ifndef DEBUG_DEBUGUI_HPP
#define DEBUG_DEBUGUI_HPP

#include <optional>
#include <string>
#include <vector>

class Engine;
class Light;
class Object;
struct MeshRenderingResource;
struct TextureRenderingResource;

class DebugUI {
private:
    Engine *_engine;

    const char *_lightKinds[3] = {
            "Point",
            "Spot",
            "Rectangle"
    };

    std::vector<const char*> _shaders;
    int _selectedShaderIdx = -1;
    int _shaderBuildResult = -1;
    std::string _shaderCode;

    std::vector<const char*> _meshes;
    std::vector<const char*> _textures;

    float _floatStep = 0.1;
    float _floatFastStep = 0.5;
    int _engineFpsWindowVisible;
    int _rendererShaderEditorVisible;
    int _sceneObjectsWindowVisible;
    int _sceneLightsWindowVisible;
    std::optional<std::vector<Object *>::const_iterator> _selectedObject = std::nullopt;
    int _selectedObjectMeshIdx = -1;
    int _selectedObjectTextureIdx = -1;
    std::optional<std::vector<Light *>::const_iterator> _selectedLight = std::nullopt;
    int _selectedLightKindIdx = -1;

    void loadShader(const char *path);
    void saveShader(const char *path);
    void buildShader(const char *path);

    void drawMainMenu();
    void drawEngineFpsWindow();
    void drawRendererShaderEditor();
    void drawSceneObjectsWindow();
    void drawSceneLightsWindow();

public:
    DebugUI(Engine *engine);

    void render();
};

#endif // DEBUG_DEBUGUI_HPP
