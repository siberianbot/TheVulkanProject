#include "Engine.hpp"

#include "Constants.hpp"
#include "Mesh.hpp"
#include "Math.hpp"
#include "Texture.hpp"
#include "Rendering/Renderpasses/SceneRenderpass.hpp"

#define GLM_FORCE_RADIANS
#define GLM_FORCE_DEPTH_ZERO_TO_ONE
#define GLM_ENABLE_EXPERIMENTAL

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

Engine::Engine()
        : renderer(this),
          _camera(glm::vec3(2, 2, -2), glm::radians(135.0f), glm::radians(50.0f)) {
    //
}

void Engine::init() {
    initGlfw();
    initWindow();

    this->renderer.init();

    glfwSetWindowUserPointer(this->_window, this);
    glfwSetFramebufferSizeCallback(_window, framebufferResizeCallback);
    glfwSetKeyCallback(_window, keyCallback);
    glfwSetCursorPosCallback(_window, cursorCallback);

    this->input.addHandler(GLFW_KEY_ESCAPE, [this](float delta) {
        glfwSetWindowShouldClose(this->_window, GLFW_TRUE);
    });

    this->input.addHandler(GLFW_KEY_W, [this](float delta) {
        this->_camera.pos() += 5 * delta * forward(this->_camera.yaw(), this->_camera.pitch());
    });
    this->input.addHandler(GLFW_KEY_S, [this](float delta) {
        this->_camera.pos() -= 5 * delta * forward(this->_camera.yaw(), this->_camera.pitch());
    });
    this->input.addHandler(GLFW_KEY_A, [this](float delta) {
        this->_camera.pos() -= 5 * delta * side(this->_camera.yaw());
    });
    this->input.addHandler(GLFW_KEY_D, [this](float delta) {
        this->_camera.pos() += 5 * delta * side(this->_camera.yaw());
    });

    this->mouseInput.addHandler([this](double dx, double dy) {
        const float sensitivity = 0.0005f;
        this->_camera.yaw() -= sensitivity * (float) dx;
        this->_camera.pitch() += sensitivity * (float) dy;

        static float pi = glm::radians(180.0f);
        static float twoPi = glm::radians(360.0f);
        const float eps = glm::radians(0.0001f);

        if (this->_camera.yaw() < 0) {
            this->_camera.yaw() += twoPi;
        } else if (this->_camera.yaw() > twoPi) {
            this->_camera.yaw() -= twoPi;
        }

        if (this->_camera.pitch() < 0) {
            this->_camera.pitch() = eps;
        } else if (this->_camera.pitch() > pi) {
            this->_camera.pitch() = pi - eps;
        }
    });

    Mesh vikingRoomMesh = readMesh("models/viking_room.obj");
    Texture vikingRoomTexture = {"textures/viking_room.png"};

    this->firstBoundMesh = this->renderer.sceneRenderpass()->uploadMesh(vikingRoomMesh, vikingRoomTexture);
    this->firstBoundMesh->model = glm::translate(glm::mat4(1.0f), glm::vec3(0.0f, -0.5f, 0.0f));

    this->secondBoundMesh = this->renderer.sceneRenderpass()->uploadMesh(vikingRoomMesh, vikingRoomTexture);
    this->secondBoundMesh->model = glm::translate(glm::mat4(1.0f), glm::vec3(-2.0f, -0.5f, 0.0f));

    this->thirdBoundMesh = this->renderer.sceneRenderpass()->uploadMesh(vikingRoomMesh, vikingRoomTexture);
    this->thirdBoundMesh->model = glm::translate(glm::mat4(1.0f), glm::vec3(0.0f, -0.5f, 2.0f));
}

void Engine::cleanup() {
    this->renderer.sceneRenderpass()->freeMesh(this->thirdBoundMesh);
    this->renderer.sceneRenderpass()->freeMesh(this->secondBoundMesh);
    this->renderer.sceneRenderpass()->freeMesh(this->firstBoundMesh);
    this->renderer.cleanup();

    if (this->_window) {
        glfwDestroyWindow(this->_window);
    }

    glfwTerminate();
}

void Engine::run() {
    float delta = 0;
    while (!glfwWindowShouldClose(this->_window)) {
        double startTime = glfwGetTime();

        glfwPollEvents();

        this->input.process(delta);
        this->renderer.render();

        delta = glfwGetTime() - startTime;
    }
}

void Engine::initGlfw() {
    if (glfwInit() != GLFW_TRUE) {
        throw std::runtime_error("glfwInit() failure");
    }
}

void Engine::initWindow() {
    glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);

    this->_window = glfwCreateWindow(1280, 720, NAME, nullptr, nullptr);
    if (!this->_window) {
        throw std::runtime_error("glfwCreateWindow() returned nullptr");
    }
}

void Engine::framebufferResizeCallback(GLFWwindow *window, int width, int height) {
    auto engine = reinterpret_cast<Engine *>(glfwGetWindowUserPointer(window));

    engine->renderer.requestResize();
}

void Engine::keyCallback(GLFWwindow *window, int key, int scancode, int action, int mods) {
    auto engine = reinterpret_cast<Engine *>(glfwGetWindowUserPointer(window));

    if (action == GLFW_PRESS) {
        engine->input.press(key);
    } else if (action == GLFW_RELEASE) {
        engine->input.release(key);
    }
}

void Engine::cursorCallback(GLFWwindow *window, double xpos, double ypos) {
    auto engine = reinterpret_cast<Engine *>(glfwGetWindowUserPointer(window));

    int w, h;
    glfwGetWindowSize(window, &w, &h);

    double xcenter = ((double) w) / 2;
    double ycenter = ((double) h) / 2;
    glfwSetCursorPos(window, xcenter, ycenter);

    engine->mouseInput.process(xcenter - xpos, ycenter - ypos);
}
