#ifndef DEBUGUI_HPP
#define DEBUGUI_HPP

#include <optional>
#include <vector>

class Engine;
class Object;

class DebugUI {
private:
    std::optional<std::vector<Object *>::const_iterator> _selectedObject = std::nullopt;
    Engine *_engine;

public:
    DebugUI(Engine *engine);

    void update();
};

#endif // DEBUGUI_HPP
