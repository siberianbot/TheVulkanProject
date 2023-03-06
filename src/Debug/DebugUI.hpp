#ifndef DEBUG_DEBUGUI_HPP
#define DEBUG_DEBUGUI_HPP

#include <optional>
#include <vector>

class Engine;
class Light;
class Object;

class DebugUI {
private:
    Engine *_engine;

    float _floatStep = 0.1;
    float _floatFastStep = 0.5;
    bool _engineFpsWindowVisible;
    bool _sceneObjectsWindowVisible;
    bool _sceneLightsWindowVisible;
    std::optional<std::vector<Object *>::const_iterator> _selectedObject = std::nullopt;
    std::optional<std::vector<Light *>::const_iterator> _selectedLight = std::nullopt;

    void renderMainMenu();
    void renderEngineFpsWindow();
    void renderSceneObjectsWindow();
    void renderSceneLightsWindow();

public:
    DebugUI(Engine *engine);

    void render();
};

#endif // DEBUG_DEBUGUI_HPP
