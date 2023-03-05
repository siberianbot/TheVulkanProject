#include "Engine.hpp"

#include <imgui.h>
#include <imgui_impl_glfw.h>

#include "Constants.hpp"
#include "src/Resources/Mesh.hpp"
#include "src/Resources/Texture.hpp"
#include "src/Scene/Object.hpp"
#include "src/Scene/Scene.hpp"
#include "src/Debug/DebugUI.hpp"
#include "src/Scene/Meshes.hpp"
#include "src/Scene/Skybox.hpp"

#include <glm/vec3.hpp>

Engine::Engine()
        : renderer(this),
          _debugUI(new DebugUI(this)) {
    this->_camera.position() = glm::vec3(2, 2, 2);
    this->_camera.yaw() = glm::radians(-135.0f);
    this->_camera.pitch() = glm::radians(45.0f);
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
        this->_camera.position() += 5 * delta * this->_camera.getForwardVector();
    });
    this->input.addPressHandler(GLFW_KEY_S, [this](float delta) {
        this->_camera.position() -= 5 * delta * this->_camera.getForwardVector();
    });
    this->input.addPressHandler(GLFW_KEY_A, [this](float delta) {
        this->_camera.position() -= 5 * delta * this->_camera.getSideVector();
    });
    this->input.addPressHandler(GLFW_KEY_D, [this](float delta) {
        this->_camera.position() += 5 * delta * this->_camera.getSideVector();
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

    Texture defaultTexture = Texture::fromFile("data/textures/default.png");
    this->_defaultTextureResource = this->renderer.getRenderingResourcesManager()->loadTextureArray(
            {
                    &defaultTexture,
                    &defaultTexture
            });

    Texture cubeTexture = Texture::fromFile("data/textures/cube.png");
    Texture cubeSpecularTexture = Texture::fromFile("data/textures/cube_specular.png");
    this->_cubeMeshResource = this->renderer.getRenderingResourcesManager()->loadMesh(CUBE_MESH.size(),
                                                                                      CUBE_MESH.data());
    this->_cubeTextureResource = this->renderer.getRenderingResourcesManager()->loadTextureArray(
            {
                    &cubeTexture,
                    &cubeSpecularTexture
            });

    Texture skyboxRightTexture = Texture::fromFile("data/textures/skybox_right.jpg");
    Texture skyboxLeftTexture = Texture::fromFile("data/textures/skybox_left.jpg");
    Texture skyboxUpTexture = Texture::fromFile("data/textures/skybox_up.jpg");
    Texture skyboxDownTexture = Texture::fromFile("data/textures/skybox_down.jpg");
    Texture skyboxFrontTexture = Texture::fromFile("data/textures/skybox_front.jpg");
    Texture skyboxBackTexture = Texture::fromFile("data/textures/skybox_back.jpg");
    this->_skyboxMeshResource = this->renderer.getRenderingResourcesManager()->loadMesh(SKYBOX_MESH.size(),
                                                                                        SKYBOX_MESH.data());
    this->_skyboxTextureResource = this->renderer.getRenderingResourcesManager()->loadTextureCube(
            {
                    &skyboxFrontTexture,
                    &skyboxBackTexture,
                    &skyboxUpTexture,
                    &skyboxDownTexture,
                    &skyboxRightTexture,
                    &skyboxLeftTexture,
            });

    Mesh suzanneMesh = Mesh::fromFile("data/models/suzanne.obj");
    this->_suzanneMeshResource = this->renderer.getRenderingResourcesManager()->loadMesh(&suzanneMesh);

    Mesh vikingRoomMesh = Mesh::fromFile("data/models/viking_room.obj");
    this->_vikingRoomMeshResource = this->renderer.getRenderingResourcesManager()->loadMesh(&vikingRoomMesh);

    Texture vikingRoomTexture = Texture::fromFile("data/textures/viking_room.png");
    this->_vikingRoomTextureResource = this->renderer.getRenderingResourcesManager()->loadTextureArray(
            {
                    &vikingRoomTexture,
                    &vikingRoomTexture,
            });

    this->_scene = new Scene(new Skybox(&this->_skyboxMeshResource, &this->_skyboxTextureResource));

    this->renderer.initRenderpasses();

    this->_scene->addObject(new Object(glm::vec3(0, 0, 2), glm::vec3(0), glm::vec3(1), &this->_vikingRoomMeshResource,
                                       &this->_vikingRoomTextureResource));

    this->_scene->addObject(new Object(glm::vec3(0), glm::vec3(0), glm::vec3(0.5f), &this->_cubeMeshResource,
                                       &this->_cubeTextureResource));

    this->_scene->addObject(new Object(glm::vec3(0, 0, -2), glm::vec3(0), glm::vec3(0.5f), &this->_suzanneMeshResource,
                                       &this->_defaultTextureResource));
}

void Engine::cleanup() {
    this->renderer.wait();

    delete this->_scene;

    this->renderer.getRenderingResourcesManager()->freeMesh(this->_vikingRoomMeshResource);
    this->renderer.getRenderingResourcesManager()->freeTexture(this->_vikingRoomTextureResource);
    this->renderer.getRenderingResourcesManager()->freeMesh(this->_suzanneMeshResource);
    this->renderer.getRenderingResourcesManager()->freeMesh(this->_cubeMeshResource);
    this->renderer.getRenderingResourcesManager()->freeTexture(this->_cubeTextureResource);
    this->renderer.getRenderingResourcesManager()->freeMesh(this->_skyboxMeshResource);
    this->renderer.getRenderingResourcesManager()->freeTexture(this->_skyboxTextureResource);
    this->renderer.getRenderingResourcesManager()->freeTexture(this->_defaultTextureResource);

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

        this->_debugUI->render();
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

void Engine::requestClose() {
    glfwSetWindowShouldClose(this->_window, GLFW_TRUE);
}
