#include "Engine.hpp"

#include <GLFW/glfw3.h>
#include <glm/vec3.hpp>
#include "subprojects/imgui-1.89.2/imgui.h"

#include "src/Engine/EngineVars.hpp"
#include "src/Events/EventQueue.hpp"
#include "src/Engine/InputProcessor.hpp"
#include "src/System/Window.hpp"

#include "src/Rendering/Renderer.hpp"
#include "src/Resources/MeshResource.hpp"
#include "src/Resources/ImageResource.hpp"
#include "src/Resources/ResourceManager.hpp"
#include "src/Objects/Camera.hpp"
#include "src/Objects/LightSource.hpp"
#include "src/Objects/Object.hpp"
#include "src/Scene/Scene.hpp"
#include "src/Scene/SceneManager.hpp"
#include "src/Debug/DebugUI.hpp"

Engine::Engine()
        : _engineVars(std::make_shared<EngineVars>()),
          _eventQueue(std::make_shared<EventQueue>()),
          _inputProcessor(std::make_shared<InputProcessor>(this->_eventQueue)),
          _window(std::make_shared<Window>(this->_engineVars, this->_eventQueue)),
          _renderer(std::make_shared<Renderer>(this)),
          _resourceManager(nullptr),
          _sceneManager(std::make_shared<SceneManager>(this->_eventQueue)),
          _debugUI(nullptr) {
    //
}

void Engine::init() {
    if (glfwInit() != GLFW_TRUE) {
        throw std::runtime_error("Failed to initilalize GLFW");
    }

    ImGui::CreateContext();
    ImGui::StyleColorsDark();

    this->_window->create();

    this->_eventQueue->addHandler([this](const Event &event) {
        if (event.type != CLOSE_REQUESTED_EVENT) {
            return;
        }

        this->_work = false;
    });

    this->_renderer->init();

    this->_resourceManager = std::make_shared<ResourceManager>(this->_renderer->rendererAllocator());
    this->_resourceManager->addDataDir("data");

    this->_renderer->initRenderpasses();

    this->_debugUI = std::make_shared<DebugUI>(this->_engineVars, this->_eventQueue, this->_resourceManager,
                                               this->_sceneManager, this->_window);

    this->_inputProcessor->addKeyboardPressHandler(GLFW_KEY_W, [this]() { this->_moveForward = true; });
    this->_inputProcessor->addKeyboardReleaseHandler(GLFW_KEY_W, [this]() { this->_moveForward = false; });

    this->_inputProcessor->addKeyboardPressHandler(GLFW_KEY_S, [this]() { this->_moveBackward = true; });
    this->_inputProcessor->addKeyboardReleaseHandler(GLFW_KEY_S, [this]() { this->_moveBackward = false; });

    this->_inputProcessor->addKeyboardPressHandler(GLFW_KEY_A, [this]() { this->_strafeLeft = true; });
    this->_inputProcessor->addKeyboardReleaseHandler(GLFW_KEY_A, [this]() { this->_strafeLeft = false; });

    this->_inputProcessor->addKeyboardPressHandler(GLFW_KEY_D, [this]() { this->_strafeRight = true; });
    this->_inputProcessor->addKeyboardReleaseHandler(GLFW_KEY_D, [this]() { this->_strafeRight = false; });

    this->_inputProcessor->addKeyboardReleaseHandler(GLFW_KEY_ESCAPE, [this]() {
        if (this->_state == NotFocused) {
            return;
        }

        this->_state = NotFocused;
        this->_window->showCursor();
    });

    this->_inputProcessor->addMouseReleaseHandler(GLFW_MOUSE_BUTTON_1, [this]() {
        if (this->_state == Focused) {
            return;
        }

        this->_state = Focused;
        this->_window->hideCursor();
    });

    this->_x = this->_window->cursorX();
    this->_y = this->_window->cursorY();
    this->_inputProcessor->addCursorMoveHandler([this](double x, double y) {
//        double dx = this->_x - x;
//        double dy = this->_y - y;
//
//        this->_x = x;
//        this->_y = y;
//
//        if (this->_state != Focused) {
//            return;
//        }
//
//        Camera *camera = this->_sceneManager->currentScene() != nullptr
//                         ? this->_sceneManager->currentScene()->camera()
//                         : nullptr;
//
//        if (camera == nullptr) {
//            return;
//        }
//
//        const float sensitivity = 0.0005f;
//        camera->yaw() -= sensitivity * (float) dx;
//        camera->pitch() += sensitivity * (float) dy;
//
//        static float pi = glm::radians(180.0f);
//        static float twoPi = glm::radians(360.0f);
//        const float eps = glm::radians(0.0001f);
//
//        if (camera->yaw() < 0) {
//            camera->yaw() += twoPi;
//        } else if (camera->yaw() > twoPi) {
//            camera->yaw() -= twoPi;
//        }
//
//        if (camera->pitch() < 0) {
//            camera->pitch() = eps;
//        } else if (camera->pitch() > pi) {
//            camera->pitch() = pi - eps;
//        }
    });

    std::shared_ptr<MeshResource> cubeMesh = this->_resourceManager->loadMesh("cube");
    std::shared_ptr<MeshResource> skyboxMesh = this->_resourceManager->loadMesh("skybox");
    std::shared_ptr<MeshResource> suzanneMesh = this->_resourceManager->loadMesh("suzanne");
    std::shared_ptr<MeshResource> vikingRoomMesh = this->_resourceManager->loadMesh("viking_room");

    std::shared_ptr<ImageResource> defaultTexture = this->_resourceManager->loadDefaultImage();
    std::shared_ptr<ImageResource> concreteTexture = this->_resourceManager->loadImage("concrete");
    std::shared_ptr<ImageResource> cubeTexture = this->_resourceManager->loadImage("cube_texture");
    std::shared_ptr<ImageResource> cubeSpecularTexture = this->_resourceManager->loadImage("cube_texture_specular");
    std::shared_ptr<ImageResource> vikingRoomTexture = this->_resourceManager->loadImage("viking_room_texture");
    std::shared_ptr<CubeImageResource> skyboxTexture = this->_resourceManager->loadCubeImage("skybox_texture");

//    Scene *scene = new Scene(this, new Skybox(skyboxMesh, skyboxTexture));
//
//    Object *object;
//
//    object = new Object();
//    object->position() = glm::vec3(0, 0, 2);
//    object->mesh() = vikingRoomMesh;
//    object->albedoTexture() = vikingRoomTexture;
//    scene->addObject(object);
//
//    object = new Object();
//    object->position() = glm::vec3(0);
//    object->scale() = glm::vec3(0.5f);
//    object->mesh() = cubeMesh;
//    object->albedoTexture() = cubeTexture;
//    object->specTexture() = cubeSpecularTexture;
//    scene->addObject(object);
//
//    object = new Object();
//    object->position() = glm::vec3(0, 0, -2);
//    object->scale() = glm::vec3(0.5f);
//    object->mesh() = suzanneMesh;
//    object->albedoTexture() = concreteTexture;
//    object->specTexture() = defaultTexture;
//    scene->addObject(object);
//
//    // down
//    object = new Object();
//    object->position() = glm::vec3(0, -4, 0);
//    object->rotation() = glm::vec3(glm::radians(270.0f), 0, 0);
//    object->scale() = glm::vec3(7, 9, 0.1);
//    object->mesh() = cubeMesh;
//    object->albedoTexture() = concreteTexture;
//    scene->addObject(object);
//
//    // front
//    object = new Object();
//    object->position() = glm::vec3(7, 0, 0);
//    object->rotation() = glm::vec3(0, glm::radians(270.0f), 0);
//    object->scale() = glm::vec3(9, 4, 0.1);
//    object->mesh() = cubeMesh;
//    object->albedoTexture() = concreteTexture;
//    scene->addObject(object);
//
//    // back
//    object = new Object();
//    object->position() = glm::vec3(-7, 0, 0);
//    object->rotation() = glm::vec3(0, glm::radians(90.0f), 0);
//    object->scale() = glm::vec3(9, 4, 0.1);
//    object->mesh() = cubeMesh;
//    object->albedoTexture() = concreteTexture;
//    scene->addObject(object);
//
//    // left
//    object = new Object();
//    object->position() = glm::vec3(0, 0, -9);
//    object->rotation() = glm::vec3(0);
//    object->scale() = glm::vec3(7, 4, 0.1);
//    object->mesh() = cubeMesh;
//    object->albedoTexture() = concreteTexture;
//    scene->addObject(object);
//
//    // right
//    object = new Object();
//    object->position() = glm::vec3(0, 0, 9);
//    object->rotation() = glm::vec3(0, glm::radians(180.0f), 0);
//    object->scale() = glm::vec3(7, 4, 0.1);
//    object->mesh() = cubeMesh;
//    object->albedoTexture() = concreteTexture;
//    scene->addObject(object);
//
//    Light *light;
//
//    light = new Light(glm::vec3(10), glm::vec3(1), 500);
//    light->enabled() = false;
//    light->kind() = RECT_LIGHT;
//    light->rotation().x = glm::radians(225.0f);
//    light->rotation().y = glm::radians(45.0f);
//    light->rect().x = 20;
//    light->rect().y = 20;
//    scene->addLight(light);
//
//    light = new Light(glm::vec3(2, 0, -2), glm::vec3(1, 0, 0), 50);
//    light->enabled() = false;
//    light->rotation().x = glm::radians(135.0f);
//    light->rotation().y = glm::radians(90.0f);
//    scene->addLight(light);
//
//    light = new Light(glm::vec3(2, 2, 0), glm::vec3(0, 1, 0), 20);
//    light->enabled() = true;
//    light->kind() = POINT_LIGHT;
//    scene->addLight(light);
//
//    light = new Light(glm::vec3(2, 0, 2), glm::vec3(0, 0, 1), 50);
//    light->enabled() = false;
//    light->rotation().x = glm::radians(225.0f);
//    light->rotation().y = glm::radians(90.0f);
//    scene->addLight(light);
//
//    scene->camera()->position() = glm::vec3(2, 2, 2);
//    scene->camera()->yaw() = glm::radians(-135.0f);
//    scene->camera()->pitch() = glm::radians(45.0f);
//
//    this->_sceneManager->setScene(scene);
}

void Engine::cleanup() {
    this->_renderer->wait();

    this->_resourceManager->removeAll();
    this->_renderer->cleanup();

    ImGui::DestroyContext();

    this->_window->destroy();

    glfwTerminate();
}

void Engine::run() {
    this->_work = true;

    while (this->_work) {
        double startTime = glfwGetTime();

        glfwPollEvents();

        this->_eventQueue->process();

        if (this->sceneManager()->currentScene() != nullptr) {
//            auto camera = this->sceneManager()->currentScene()->camera();
//
//            auto forward = camera->getForwardVector();
//            auto side = camera->getSideVector();
//
//            if (this->_moveForward) {
//                camera->position() += 5 * this->_delta * forward;
//            }
//
//            if (this->_moveBackward) {
//                camera->position() -= 5 * this->_delta * forward;
//            }
//
//            if (this->_strafeLeft) {
//                camera->position() -= 5 * this->_delta * side;
//            }
//
//            if (this->_strafeRight) {
//                camera->position() += 5 * this->_delta * side;
//            }
        }

        this->_debugUI->render();
        this->_renderer->render();

        this->_delta = glfwGetTime() - startTime;
    }
}
