#ifndef ENGINE_HPP
#define ENGINE_HPP

#include <cstdint>
#include <vector>

struct GLFWwindow;
class EngineVars;
class DebugUI;
class Scene;
class EventQueue;
class ResourceManager;
class SceneManager;
class Renderer;
class Input;
class MouseInput;

enum CameraControlState {
    NotFocused,
    Focused
};

class Engine {
private:
    GLFWwindow *_window = nullptr;

    Renderer *_renderer;
    Input *_input;
    MouseInput *_mouseInput;
    EventQueue *_eventQueue;
    EngineVars *_engineVars;
    ResourceManager *_resourceManager;
    SceneManager *_sceneManager;
    DebugUI *_debugUI;

    int _windowWidth = 1280;
    int _windowHeight = 720;
    float _delta = 0;
    CameraControlState _state = NotFocused;

    void initGlfw();
    void initWindow();

    static void mouseButtonCallback(GLFWwindow* window, int button, int action, int mods);
    static void framebufferResizeCallback(GLFWwindow *window, int width, int height);
    static void keyCallback(GLFWwindow *window, int key, int scancode, int action, int mods);
    static void cursorCallback(GLFWwindow *window, double xpos, double ypos);

public:
    Engine();

    void init();
    void cleanup();

    void run();

    [[nodiscard]] GLFWwindow *window() { return this->_window; }

    [[nodiscard]] Renderer *renderer() { return this->_renderer; }
    [[nodiscard]] EventQueue *eventQueue() { return this->_eventQueue; }
    [[nodiscard]] EngineVars *engineVars() { return this->_engineVars; }
    [[nodiscard]] ResourceManager *resourceManager() { return this->_resourceManager; }
    [[nodiscard]] SceneManager *sceneManager() { return this->_sceneManager; }

    [[nodiscard]] uint32_t windowWidth() { return this->_windowWidth; }
    [[nodiscard]] uint32_t windowHeight() { return this->_windowHeight; }
    [[nodiscard]] float delta() { return this->_delta; }
};

#endif // ENGINE_HPP
