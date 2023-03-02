#ifndef DEBUG_DEBUGUI_HPP
#define DEBUG_DEBUGUI_HPP

#include <optional>
#include <vector>

class Engine;
class Object;

class DebugUI {
private:
    std::optional<std::vector<Object *>::const_iterator> _selectedObject = std::nullopt;
    Engine *_engine;

    bool _objectsWindowVisible;
    bool _fpsWindowVisible;

    void renderMainMenu();
    void renderObjectsWindow();
    void renderFpsWindow();

public:
    DebugUI(Engine *engine);

    void render();
};

#endif // DEBUG_DEBUGUI_HPP
