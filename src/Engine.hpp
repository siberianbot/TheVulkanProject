#ifndef ENGINE_HPP
#define ENGINE_HPP

#include <vector>

#include "Input.hpp"
#include "Camera.hpp"
#include "src/Rendering/Renderer.hpp"
#include "src/Rendering/RenderingResourcesManager.hpp"

class Object;

class Engine {
private:
    GLFWwindow *_window = nullptr;

    Renderer renderer;
    Input input;
    MouseInput mouseInput;

    MeshRenderingResource _meshResource;
    TextureRenderingResource _textureResource;

    Camera _camera;
    std::vector<Object *> _objects;

    void initGlfw();
    void initWindow();

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

    [[nodiscard]] const std::vector<Object *> &objects() const { return this->_objects; }
};

#endif // ENGINE_HPP
