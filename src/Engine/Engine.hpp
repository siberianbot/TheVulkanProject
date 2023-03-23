#ifndef ENGINE_HPP
#define ENGINE_HPP

#include <memory>
#include <vector>

class EventQueue;
class EngineVars;
class InputProcessor;
class Window;
class RenderingManager;
class ResourceManager;
class SceneManager;
class Renderer;
class DebugUI;

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
    std::shared_ptr<RenderingManager> _renderingManager;
    std::shared_ptr<ResourceManager> _resourceManager;
    std::shared_ptr<SceneManager> _sceneManager;
    std::shared_ptr<Renderer> _renderer;
    std::shared_ptr<DebugUI> _debugUI;

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

    [[nodiscard]] const std::shared_ptr<EngineVars> engineVars() const { return this->_engineVars; }
    [[nodiscard]] const std::shared_ptr<EventQueue> eventQueue() const { return this->_eventQueue; }
    [[nodiscard]] const std::shared_ptr<InputProcessor> inputProcessor() const { return this->_inputProcessor; }
    [[nodiscard]] const std::shared_ptr<Window> window() const { return this->_window; }
    [[nodiscard]] const std::shared_ptr<RenderingManager> renderingManager() const { return this->_renderingManager; }
    [[nodiscard]] const std::shared_ptr<ResourceManager> resourceManager() const { return this->_resourceManager; }
    [[nodiscard]] const std::shared_ptr<SceneManager> sceneManager() const { return this->_sceneManager; }
    [[nodiscard]] const std::shared_ptr<Renderer> renderer() const { return this->_renderer; }

    [[nodiscard]] float delta() { return this->_delta; }
};

#endif // ENGINE_HPP
