#ifndef ENGINE_HPP
#define ENGINE_HPP

#include <memory>
#include <vector>

class Log;
class EventQueue;
class VarCollection;
class InputProcessor;
class Window;
class RenderingManager;
class ResourceManager;
class SceneManager;
class Renderer;
class DebugUIRoot;

enum CameraControlState {
    NotFocused,
    Focused
};

class Engine {
private:
    std::shared_ptr<Log> _log;
    std::shared_ptr<VarCollection> _vars;
    std::shared_ptr<EventQueue> _eventQueue;
    std::shared_ptr<InputProcessor> _inputProcessor;
    std::shared_ptr<Window> _window;
    std::shared_ptr<RenderingManager> _renderingManager;
    std::shared_ptr<ResourceManager> _resourceManager;
    std::shared_ptr<SceneManager> _sceneManager;
    std::shared_ptr<Renderer> _renderer;
    std::shared_ptr<DebugUIRoot> _debugUI;

    bool _work = false;
    float _delta = 0;
    CameraControlState _state = NotFocused;
    double _x = 0;
    double _y = 0;
    bool _moveForward = false;
    bool _moveBackward = false;
    bool _strafeLeft = false;
    bool _strafeRight = false;

public:
    Engine();

    void init();
    void cleanup();

    void run();
};

#endif // ENGINE_HPP
