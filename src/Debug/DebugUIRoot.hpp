#ifndef DEBUG_DEBUGUIROOT_HPP
#define DEBUG_DEBUGUIROOT_HPP

#include <map>
#include <memory>
#include <string>
#include <vector>

class Log;
class VarCollection;
class EventQueue;
class ResourceDatabase;
class SceneManager;

struct DebugUIState;
class LogWindow;
class MainMenuBar;
class ObjectEditorWindow;
class ResourcesListWindow;
class SceneTreeWindow;
class ShaderCodeEditorWindow;
class VariablesWindow;

class DebugUIRoot {
private:
    std::shared_ptr<DebugUIState> _state;

    std::shared_ptr<LogWindow> _logWindow;
    std::shared_ptr<MainMenuBar> _mainMenuBar;
    std::shared_ptr<ObjectEditorWindow> _objectEditorWindow;
    std::shared_ptr<ResourcesListWindow> _resourceListWindow;
    std::shared_ptr<SceneTreeWindow> _sceneTreeWindow;
    std::shared_ptr<ShaderCodeEditorWindow> _shaderCodeEditorWindow;
    std::shared_ptr<VariablesWindow> _variablesWindow;

public:
    DebugUIRoot(const std::shared_ptr<Log> &log,
                const std::shared_ptr<EventQueue> &eventQueue,
                const std::shared_ptr<VarCollection> &vars,
                const std::shared_ptr<ResourceDatabase> &resourceDatabase,
                const std::shared_ptr<SceneManager> &sceneManager);

    void render();
};

#endif // DEBUG_DEBUGUIROOT_HPP
