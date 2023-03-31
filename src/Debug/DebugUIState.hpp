#ifndef DEBUG_DEBUGUISTATE_HPP
#define DEBUG_DEBUGUISTATE_HPP

#include <memory>

class Object;

struct DebugUIState {
    bool logWindowVisible = false;
    bool objectEditorWindowVisible = false;
    bool resourceListWindowVisible = false;
    bool sceneTreeWindowVisible = false;
    bool shaderCodeEditorWindowVisible = false;
    bool variablesWindowVisible = false;

    std::weak_ptr<Object> object;
};

#endif // DEBUG_DEBUGUISTATE_HPP
