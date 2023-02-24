#include "Engine.hpp"

#include <imgui.h>
#include <imgui_impl_glfw.h>

#include "Constants.hpp"
#include "src/Resources/Mesh.hpp"
#include "Math.hpp"
#include "src/Resources/Texture.hpp"
#include "src/Scene/Object.hpp"
#include "src/Scene/Scene.hpp"
#include "DebugUI.hpp"

#include <glm/vec3.hpp>

Engine::Engine()
        : renderer(this),
          _camera(glm::vec3(2, 2, -2), glm::radians(135.0f), glm::radians(50.0f)),
          _scene(new Scene()),
          _debugUI(new DebugUI(this)) {
    //
}

void Engine::init() {
    initGlfw();
    initWindow();

    glfwSetWindowUserPointer(this->_window, this);
    glfwSetFramebufferSizeCallback(_window, framebufferResizeCallback);
    glfwSetKeyCallback(_window, keyCallback);
    glfwSetMouseButtonCallback(_window, mouseButtonCallback);
    glfwSetCursorPosCallback(_window, cursorCallback);

    ImGui::CreateContext();
    ImGui::StyleColorsDark();
    ImGui_ImplGlfw_InitForVulkan(this->_window, true);

    this->renderer.init();

    this->input.addReleaseHandler(GLFW_KEY_ESCAPE, [this](float delta) {
        switch (this->_state) {
            case NotFocused:
                glfwSetWindowShouldClose(this->_window, GLFW_TRUE);
                break;

            case Focused:
                this->_state = NotFocused;
                glfwSetInputMode(this->_window, GLFW_CURSOR, GLFW_CURSOR_NORMAL);
                break;
        }
    });

    this->input.addPressHandler(GLFW_KEY_W, [this](float delta) {
        this->_camera.pos() += 5 * delta * forward(this->_camera.yaw(), this->_camera.pitch());
    });
    this->input.addPressHandler(GLFW_KEY_S, [this](float delta) {
        this->_camera.pos() -= 5 * delta * forward(this->_camera.yaw(), this->_camera.pitch());
    });
    this->input.addPressHandler(GLFW_KEY_A, [this](float delta) {
        this->_camera.pos() -= 5 * delta * side(this->_camera.yaw());
    });
    this->input.addPressHandler(GLFW_KEY_D, [this](float delta) {
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

    Mesh vikingRoomMesh = Mesh::fromFile("data/models/viking_room.obj");
    Texture vikingRoomTexture = Texture::fromFile("data/textures/viking_room.png");

    this->_meshResource = this->renderer.getRenderingResourcesManager()->loadMesh(&vikingRoomMesh);
    this->_textureResource = this->renderer.getRenderingResourcesManager()->loadTexture(&vikingRoomTexture);
}

void Engine::cleanup() {
    this->renderer.wait();

    delete this->_scene;

    this->renderer.getRenderingResourcesManager()->freeMesh(this->_meshResource);
    this->renderer.getRenderingResourcesManager()->freeTexture(this->_textureResource);

    this->renderer.cleanup();

    ImGui::DestroyContext();

    if (this->_window) {
        glfwDestroyWindow(this->_window);
    }

    glfwTerminate();
}

void Engine::run() {
    while (!glfwWindowShouldClose(this->_window)) {
        double startTime = glfwGetTime();

        glfwPollEvents();

        this->_debugUI->update();
        this->input.process(this->_delta);
        this->renderer.render();

        this->_delta = glfwGetTime() - startTime;
    }
}

void Engine::initGlfw() {
    if (glfwInit() != GLFW_TRUE) {
        throw std::runtime_error("glfwInit() failure");
    }
}

void Engine::initWindow() {
    glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);

    this->_window = glfwCreateWindow(this->_windowWidth, this->_windowHeight, NAME, nullptr, nullptr);
    if (!this->_window) {
        throw std::runtime_error("glfwCreateWindow() returned nullptr");
    }
}

void Engine::mouseButtonCallback(GLFWwindow *window, int button, int action, int mods) {
    auto engine = reinterpret_cast<Engine *>(glfwGetWindowUserPointer(window));

    if (ImGui::GetIO().WantCaptureMouse) {
        return;
    }

    if (engine->_state == NotFocused && action == GLFW_RELEASE) {
        engine->_state = Focused;
        glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);

        int w, h;
        glfwGetWindowSize(engine->_window, &w, &h);

        double xcenter = ((double) w) / 2;
        double ycenter = ((double) h) / 2;
        glfwSetCursorPos(engine->_window, xcenter, ycenter);
    }
}

void Engine::framebufferResizeCallback(GLFWwindow *window, int width, int height) {
    auto engine = reinterpret_cast<Engine *>(glfwGetWindowUserPointer(window));

    engine->_windowWidth = width;
    engine->_windowHeight = height;
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

    if (engine->_state == NotFocused) {
        return;
    }

    int w, h;
    glfwGetWindowSize(window, &w, &h);

    double xcenter = ((double) w) / 2;
    double ycenter = ((double) h) / 2;
    glfwSetCursorPos(window, xcenter, ycenter);

    engine->mouseInput.process(xcenter - xpos, ycenter - ypos);
}
