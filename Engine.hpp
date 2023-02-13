#ifndef ENGINE_HPP
#define ENGINE_HPP

#include "Renderer.hpp"
#include "Input.hpp"
#include "Camera.hpp"

class Engine {
private:
    GLFWwindow *_window = nullptr;

    Renderer renderer;
    Input input;
    MouseInput mouseInput;

    BoundMeshInfo *firstBoundMesh;
    BoundMeshInfo *secondBoundMesh;
    BoundMeshInfo *thirdBoundMesh;

    Camera _camera;

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
};

#endif // ENGINE_HPP
