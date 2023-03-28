#ifndef DEBUG_DEBUGUIROOT_HPP
#define DEBUG_DEBUGUIROOT_HPP

#include <map>
#include <memory>
#include <string>
#include <vector>

struct DebugUIState;
class MainMenuBar;
class ResourcesListWindow;
class SceneTreeWindow;
class ShaderCodeEditorWindow;
class VariablesWindow;
class VarCollection;
class EventQueue;
class ResourceManager;
class SceneManager;

class DebugUIRoot {
private:
    std::shared_ptr<DebugUIState> _state;

    std::shared_ptr<MainMenuBar> _mainMenuBar;
    std::shared_ptr<ResourcesListWindow> _resourceListWindow;
    std::shared_ptr<SceneTreeWindow> _sceneTreeWindow;
    std::shared_ptr<ShaderCodeEditorWindow> _shaderCodeEditorWindow;
    std::shared_ptr<VariablesWindow> _variablesWindow;

public:
    DebugUIRoot(const std::shared_ptr<EventQueue> &eventQueue,
                const std::shared_ptr<VarCollection> &vars,
                const std::shared_ptr<ResourceManager> &resourceManager,
                const std::shared_ptr<SceneManager> &sceneManager);

    void render();
};

#endif // DEBUG_DEBUGUIROOT_HPP
