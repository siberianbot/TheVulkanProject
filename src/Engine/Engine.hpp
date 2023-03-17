#ifndef ENGINE_HPP
#define ENGINE_HPP

#include <memory>
#include <vector>

class EventQueue;
class EngineVars;
class InputProcessor;
class Window;

class DebugUI;
class Scene;
class ResourceManager;
class SceneManager;
class Renderer;

enum CameraControlState {
    NotFocused,
    Focused
};

class Engine {
private:
    std::shared_ptr<EngineVars> _engineVars;
    std::shared_ptr<EventQueue> _eventQueue;
    std::shared_ptr<InputProcessor> _inputProcessor;
    std::shared_ptr<Window> _window;

    Renderer *_renderer;
    ResourceManager *_resourceManager;
    SceneManager *_sceneManager;
    DebugUI *_debugUI;

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

    [[nodiscard]] std::shared_ptr<EngineVars> engineVars() const { return this->_engineVars; }
    [[nodiscard]] std::shared_ptr<EventQueue> eventQueue() const { return this->_eventQueue; }
    [[nodiscard]] std::shared_ptr<InputProcessor> inputProcessor() const { return this->_inputProcessor; }
    [[nodiscard]] std::shared_ptr<Window> window() const { return this->_window; }

    [[nodiscard]] Renderer *renderer() { return this->_renderer; }
    [[nodiscard]] ResourceManager *resourceManager() { return this->_resourceManager; }
    [[nodiscard]] SceneManager *sceneManager() { return this->_sceneManager; }

    [[nodiscard]] float delta() { return this->_delta; }
};

#endif // ENGINE_HPP
