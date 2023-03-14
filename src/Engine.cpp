#include "Engine.hpp"

#include <GLFW/glfw3.h>
#include <glm/vec3.hpp>
#include <imgui.h>
#include <imgui_impl_glfw.h>

#include "Constants.hpp"
#include "EngineVars.hpp"
#include "Input.hpp"
#include "src/Events/EventQueue.hpp"
#include "src/Rendering/Renderer.hpp"
#include "src/Resources/MeshResource.hpp"
#include "src/Resources/ImageResource.hpp"
#include "src/Resources/ResourceManager.hpp"
#include "src/Objects/Camera.hpp"
#include "src/Objects/Light.hpp"
#include "src/Objects/Object.hpp"
#include "src/Scene/Scene.hpp"
#include "src/Debug/DebugUI.hpp"
#include "src/Objects/Skybox.hpp"
#include "src/Scene/SceneManager.hpp"

Engine::Engine()
        : _renderer(new Renderer(this)),
          _input(new Input()),
          _mouseInput(new MouseInput()),
          _eventQueue(new EventQueue()),
          _engineVars(EngineVars::defaults()),
          _resourceManager(nullptr),
          _sceneManager(new SceneManager(this->_eventQueue)) {
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

    this->_eventQueue->addHandler([this](const Event &event) {
        if (event.type != CLOSE_REQUESTED_EVENT) {
            return;
        }

        glfwSetWindowShouldClose(this->_window, GLFW_TRUE);
    });

    this->_renderer->init();

    this->_resourceManager = new ResourceManager(this->_renderer->rendererAllocator());
    this->_resourceManager->addDataDir("data");

    this->_renderer->initRenderpasses();

    this->_input->addReleaseHandler(GLFW_KEY_ESCAPE, [this](float delta) {
        if (this->_state != Focused) {
            return;
        }

        this->_state = NotFocused;
        glfwSetInputMode(this->_window, GLFW_CURSOR, GLFW_CURSOR_NORMAL);
    });

    this->_input->addPressHandler(GLFW_KEY_W, [this](float delta) {
        Camera *camera = this->_sceneManager->currentScene() != nullptr
                         ? this->_sceneManager->currentScene()->camera()
                         : nullptr;

        if (camera == nullptr) {
            return;
        }

        camera->position() += 5 * delta * camera->getForwardVector();
    });
    this->_input->addPressHandler(GLFW_KEY_S, [this](float delta) {
        Camera *camera = this->_sceneManager->currentScene() != nullptr
                         ? this->_sceneManager->currentScene()->camera()
                         : nullptr;

        if (camera == nullptr) {
            return;
        }

        camera->position() -= 5 * delta * camera->getForwardVector();
    });
    this->_input->addPressHandler(GLFW_KEY_A, [this](float delta) {
        Camera *camera = this->_sceneManager->currentScene() != nullptr
                         ? this->_sceneManager->currentScene()->camera()
                         : nullptr;

        if (camera == nullptr) {
            return;
        }

        camera->position() -= 5 * delta * camera->getSideVector();
    });
    this->_input->addPressHandler(GLFW_KEY_D, [this](float delta) {
        Camera *camera = this->_sceneManager->currentScene() != nullptr
                         ? this->_sceneManager->currentScene()->camera()
                         : nullptr;

        if (camera == nullptr) {
            return;
        }

        camera->position() += 5 * delta * camera->getSideVector();
    });

    this->_mouseInput->addHandler([this](double dx, double dy) {
        Camera *camera = this->_sceneManager->currentScene() != nullptr
                         ? this->_sceneManager->currentScene()->camera()
                         : nullptr;

        if (camera == nullptr) {
            return;
        }

        const float sensitivity = 0.0005f;
        camera->yaw() -= sensitivity * (float) dx;
        camera->pitch() += sensitivity * (float) dy;

        static float pi = glm::radians(180.0f);
        static float twoPi = glm::radians(360.0f);
        const float eps = glm::radians(0.0001f);

        if (camera->yaw() < 0) {
            camera->yaw() += twoPi;
        } else if (camera->yaw() > twoPi) {
            camera->yaw() -= twoPi;
        }

        if (camera->pitch() < 0) {
            camera->pitch() = eps;
        } else if (camera->pitch() > pi) {
            camera->pitch() = pi - eps;
        }
    });

    MeshResource *cubeMesh = this->_resourceManager->loadMesh("cube");
    MeshResource *skyboxMesh = this->_resourceManager->loadMesh("skybox");
    MeshResource *suzanneMesh = this->_resourceManager->loadMesh("suzanne");
    MeshResource *vikingRoomMesh = this->_resourceManager->loadMesh("viking_room");

    ImageResource *defaultTexture = this->_resourceManager->loadDefaultImage();
    ImageResource *concreteTexture = this->_resourceManager->loadImage("concrete");
    ImageResource *cubeTexture = this->_resourceManager->loadImage("cube_texture");
    ImageResource *cubeSpecularTexture = this->_resourceManager->loadImage("cube_texture_specular");
    ImageResource *vikingRoomTexture = this->_resourceManager->loadImage("viking_room_texture");
    CubeImageResource *skyboxTexture = this->_resourceManager->loadCubeImage("skybox_texture");

    Scene *scene = new Scene(this, new Skybox(skyboxMesh, skyboxTexture));

    Object *object;

    object = new Object();
    object->position() = glm::vec3(0, 0, 2);
    object->mesh() = vikingRoomMesh;
    object->albedoTexture() = vikingRoomTexture;
    scene->addObject(object);

    object = new Object();
    object->position() = glm::vec3(0);
    object->scale() = glm::vec3(0.5f);
    object->mesh() = cubeMesh;
    object->albedoTexture() = cubeTexture;
    object->specTexture() = cubeSpecularTexture;
    scene->addObject(object);

    object = new Object();
    object->position() = glm::vec3(0, 0, -2);
    object->scale() = glm::vec3(0.5f);
    object->mesh() = suzanneMesh;
    object->albedoTexture() = concreteTexture;
    object->specTexture() = defaultTexture;
    scene->addObject(object);

    // down
    object = new Object();
    object->position() = glm::vec3(0, -4, 0);
    object->rotation() = glm::vec3(glm::radians(270.0f), 0, 0);
    object->scale() = glm::vec3(7, 9, 0.1);
    object->mesh() = cubeMesh;
    object->albedoTexture() = concreteTexture;
    scene->addObject(object);

    // front
    object = new Object();
    object->position() = glm::vec3(7, 0, 0);
    object->rotation() = glm::vec3(0, glm::radians(270.0f), 0);
    object->scale() = glm::vec3(9, 4, 0.1);
    object->mesh() = cubeMesh;
    object->albedoTexture() = concreteTexture;
    scene->addObject(object);

    // back
    object = new Object();
    object->position() = glm::vec3(-7, 0, 0);
    object->rotation() = glm::vec3(0, glm::radians(90.0f), 0);
    object->scale() = glm::vec3(9, 4, 0.1);
    object->mesh() = cubeMesh;
    object->albedoTexture() = concreteTexture;
    scene->addObject(object);

    // left
    object = new Object();
    object->position() = glm::vec3(0, 0, -9);
    object->rotation() = glm::vec3(0);
    object->scale() = glm::vec3(7, 4, 0.1);
    object->mesh() = cubeMesh;
    object->albedoTexture() = concreteTexture;
    scene->addObject(object);

    // right
    object = new Object();
    object->position() = glm::vec3(0, 0, 9);
    object->rotation() = glm::vec3(0, glm::radians(180.0f), 0);
    object->scale() = glm::vec3(7, 4, 0.1);
    object->mesh() = cubeMesh;
    object->albedoTexture() = concreteTexture;
    scene->addObject(object);

    Light *light;

    light = new Light(glm::vec3(10), glm::vec3(1), 500);
    light->enabled() = false;
    light->kind() = RECT_LIGHT;
    light->rotation().x = glm::radians(225.0f);
    light->rotation().y = glm::radians(45.0f);
    light->rect().x = 20;
    light->rect().y = 20;
    scene->addLight(light);

    light = new Light(glm::vec3(2, 0, -2), glm::vec3(1, 0, 0), 50);
    light->enabled() = false;
    light->rotation().x = glm::radians(135.0f);
    light->rotation().y = glm::radians(90.0f);
    scene->addLight(light);

    light = new Light(glm::vec3(2, 2, 0), glm::vec3(0, 1, 0), 20);
    light->enabled() = true;
    light->kind() = POINT_LIGHT;
    scene->addLight(light);

    light = new Light(glm::vec3(2, 0, 2), glm::vec3(0, 0, 1), 50);
    light->enabled() = false;
    light->rotation().x = glm::radians(225.0f);
    light->rotation().y = glm::radians(90.0f);
    scene->addLight(light);

    this->_debugUI = new DebugUI(this);

    scene->camera()->position() = glm::vec3(2, 2, 2);
    scene->camera()->yaw() = glm::radians(-135.0f);
    scene->camera()->pitch() = glm::radians(45.0f);

    this->_sceneManager->switchScene(scene);
}

void Engine::cleanup() {
    this->_renderer->wait();

    this->_resourceManager->unloadAll();
    this->_renderer->cleanup();

    ImGui::DestroyContext();

    if (this->_window) {
        glfwDestroyWindow(this->_window);
    }

    glfwTerminate();

    delete this->_debugUI;
    delete this->_sceneManager;
    delete this->_resourceManager;
    delete this->_engineVars;
    delete this->_eventQueue;
    delete this->_mouseInput;
    delete this->_input;
}

void Engine::run() {
    while (!glfwWindowShouldClose(this->_window)) {
        double startTime = glfwGetTime();

        this->_eventQueue->process();

        glfwPollEvents();

        this->_debugUI->render();
        this->_input->process(this->_delta);
        this->_renderer->render();

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

    engine->_eventQueue->pushEvent(Event{
            .type = VIEWPORT_RESIZED_EVENT,
            .viewport = {
                    .width = static_cast<uint32_t>(width),
                    .height = static_cast<uint32_t>(height)
            }
    });
}

void Engine::keyCallback(GLFWwindow *window, int key, int scancode, int action, int mods) {
    auto engine = reinterpret_cast<Engine *>(glfwGetWindowUserPointer(window));

    if (ImGui::GetIO().WantCaptureKeyboard) {
        return;
    }

    if (action == GLFW_PRESS) {
        engine->_input->press(key);
    } else if (action == GLFW_RELEASE) {
        engine->_input->release(key);
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

    engine->_mouseInput->process(xcenter - xpos, ycenter - ypos);
}
