#ifndef ENGINE_HPP
#define ENGINE_HPP

#include <vector>

#include "Input.hpp"
#include "src/Objects/Camera.hpp"
#include "src/Rendering/Renderer.hpp"
#include "src/Rendering/RenderingResourcesManager.hpp"

class EngineVars;
class DebugUI;
class Object;
class Scene;
class EventQueue;
class ResourceManager;

enum CameraControlState {
    NotFocused,
    Focused
};

class Engine {
private:
    GLFWwindow *_window = nullptr;

    Renderer renderer;
    Input input;
    MouseInput mouseInput;

    EventQueue *_eventQueue;
    EngineVars *_engineVars;
    ResourceManager *_resourceManager;

    std::vector<MeshRenderingResource> _meshes;
    std::vector<TextureRenderingResource> _textures;

    MeshRenderingResource _skyboxMeshResource;
    TextureRenderingResource _skyboxTextureResource;
    TextureRenderingResource _defaultTextureResource;

    int _windowWidth = 1280;
    int _windowHeight = 720;
    float _delta = 0;
    CameraControlState _state = NotFocused;
    Camera _camera;
    Scene *_scene;
    DebugUI *_debugUI;

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

    GLFWwindow *window() { return this->_window; }
    Camera &camera() { return this->_camera; }

    [[nodiscard]] EventQueue *eventQueue() { return this->_eventQueue; }
    [[nodiscard]] EngineVars *engineVars() { return this->_engineVars; }
    [[nodiscard]] ResourceManager *resourceManager() { return this->_resourceManager; }
    [[nodiscard]] Scene *scene() { return this->_scene; }

    uint32_t windowWidth() { return this->_windowWidth; }
    uint32_t windowHeight() { return this->_windowHeight; }
    float delta() { return this->_delta; }

    TextureRenderingResource *defaultTextureResource() { return &this->_defaultTextureResource; }

    std::vector<MeshRenderingResource> &meshes() { return this->_meshes; }
    std::vector<TextureRenderingResource> &textures() { return this->_textures; }
};

#endif // ENGINE_HPP
