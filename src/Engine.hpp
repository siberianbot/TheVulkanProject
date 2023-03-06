#ifndef ENGINE_HPP
#define ENGINE_HPP

#include <vector>

#include "Input.hpp"
#include "src/Scene/Camera.hpp"
#include "src/Rendering/Renderer.hpp"
#include "src/Rendering/RenderingResourcesManager.hpp"

class DebugUI;
class Object;
class Scene;

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

    MeshRenderingResource _skyboxMeshResource;
    TextureRenderingResource _skyboxTextureResource;
    TextureRenderingResource _defaultTextureResource;
    MeshRenderingResource _cubeMeshResource;
    TextureRenderingResource _cubeTextureResource;
    MeshRenderingResource _suzanneMeshResource;
    MeshRenderingResource _vikingRoomMeshResource;
    TextureRenderingResource _vikingRoomTextureResource;
    MeshRenderingResource _planeMeshResource;
    TextureRenderingResource _concreteTextureResource;

    int _windowWidth = 1280;
    int _windowHeight = 720;
    float _delta = 0;
    CameraControlState _state = NotFocused;
    Camera _camera;
    Scene *_scene;
    DebugUI *_debugUI;

    bool _rendererReloadRequested = false;

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

    [[nodiscard]] Scene *scene() { return this->_scene; }

    uint32_t windowWidth() { return this->_windowWidth; }
    uint32_t windowHeight() { return this->_windowHeight; }
    float delta() { return this->_delta; }
    MeshRenderingResource *cubeMeshResource() { return &this->_cubeMeshResource; }
    TextureRenderingResource *cubeTextureResource() { return &this->_cubeTextureResource; }
    TextureRenderingResource *defaultTextureResource() { return &this->_defaultTextureResource; }

    void requestClose();
    void requestRendererReload();
};

#endif // ENGINE_HPP
