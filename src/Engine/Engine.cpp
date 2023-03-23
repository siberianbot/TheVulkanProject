#include "Engine.hpp"

#include <GLFW/glfw3.h>
#include "subprojects/imgui-1.89.2/imgui.h"

#include "src/Engine/EngineVars.hpp"
#include "src/Events/EventQueue.hpp"
#include "src/Engine/InputProcessor.hpp"
#include "src/System/Window.hpp"
#include "src/Rendering/RenderingManager.hpp"
#include "src/Resources/MeshResource.hpp"
#include "src/Resources/ImageResource.hpp"
#include "src/Resources/ResourceManager.hpp"
#include "src/Objects/Camera.hpp"
#include "src/Objects/LightSource.hpp"
#include "src/Objects/Prop.hpp"
#include "src/Objects/World.hpp"
#include "src/Objects/Components/ModelComponent.hpp"
#include "src/Objects/Components/PositionComponent.hpp"
#include "src/Objects/Components/SkyboxComponent.hpp"
#include "src/Scene/Scene.hpp"
#include "src/Scene/SceneNode.hpp"
#include "src/Scene/SceneManager.hpp"
#include "src/Debug/DebugUI.hpp"

Engine::Engine()
        : _engineVars(std::make_shared<EngineVars>()),
          _eventQueue(std::make_shared<EventQueue>()),
          _inputProcessor(std::make_shared<InputProcessor>(this->_eventQueue)),
          _window(std::make_shared<Window>(this->_engineVars, this->_eventQueue)),
          _renderingManager(nullptr),
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

    this->_renderingManager = std::make_shared<RenderingManager>(this->_engineVars, this->_window);
    this->_renderingManager->init();

    this->_resourceManager = std::make_shared<ResourceManager>(this->_renderingManager->renderingObjectsAllocator());
    this->_resourceManager->addDataDir("data");

//  TODO:  this->_renderer->initRenderpasses();

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

    this->_sceneManager->setScene(Scene::empty());

    std::shared_ptr<World> world = std::make_shared<World>();
    world->skybox()->setMesh(skyboxMesh);
    world->skybox()->setTexture(skyboxTexture);
    this->_sceneManager->currentScene()->root()->object() = world;

    std::shared_ptr<Prop> prop;

    prop = std::make_shared<Prop>();
    prop->position()->position = glm::vec3(0, 0, 2);
    prop->model()->setMesh(vikingRoomMesh);
    prop->model()->setAlbedoTexture(vikingRoomTexture);
    this->_sceneManager->addObject(prop);

    prop = std::make_shared<Prop>();
    prop->position()->position = glm::vec3(0);
    prop->position()->scale = glm::vec3(0.5f);
    prop->model()->setMesh(cubeMesh);
    prop->model()->setAlbedoTexture(cubeTexture);
    prop->model()->setSpecularTexture(cubeSpecularTexture);
    this->_sceneManager->addObject(prop);

    prop = std::make_shared<Prop>();
    prop->position()->position = glm::vec3(0, 0, -2);
    prop->position()->scale = glm::vec3(0.5f);
    prop->model()->setMesh(suzanneMesh);
    prop->model()->setAlbedoTexture(concreteTexture);
    prop->model()->setSpecularTexture(defaultTexture);
    this->_sceneManager->addObject(prop);

    // down
    prop = std::make_shared<Prop>();
    prop->position()->position = glm::vec3(0, -4, 0);
    prop->position()->rotation = glm::vec3(glm::radians(270.0f), 0, 0);
    prop->position()->scale = glm::vec3(7, 9, 0.1);
    prop->model()->setMesh(cubeMesh);
    prop->model()->setAlbedoTexture(concreteTexture);
    this->_sceneManager->addObject(prop);

    // front
    prop = std::make_shared<Prop>();
    prop->position()->position = glm::vec3(7, 0, 0);
    prop->position()->rotation = glm::vec3(0, glm::radians(270.0f), 0);
    prop->position()->scale = glm::vec3(9, 4, 0.1);
    prop->model()->setMesh(cubeMesh);
    prop->model()->setAlbedoTexture(concreteTexture);
    this->_sceneManager->addObject(prop);

    // back
    prop = std::make_shared<Prop>();
    prop->position()->position = glm::vec3(-7, 0, 0);
    prop->position()->rotation = glm::vec3(0, glm::radians(90.0f), 0);
    prop->position()->scale = glm::vec3(9, 4, 0.1);
    prop->model()->setMesh(cubeMesh);
    prop->model()->setAlbedoTexture(concreteTexture);
    this->_sceneManager->addObject(prop);

    // left
    prop = std::make_shared<Prop>();
    prop->position()->position = glm::vec3(0, 0, -9);
    prop->position()->rotation = glm::vec3(0);
    prop->position()->scale = glm::vec3(7, 4, 0.1);
    prop->model()->setMesh(cubeMesh);
    prop->model()->setAlbedoTexture(concreteTexture);
    this->_sceneManager->addObject(prop);

    // right
    prop = std::make_shared<Prop>();
    prop->position()->position = glm::vec3(0, 0, 9);
    prop->position()->rotation = glm::vec3(0, glm::radians(180.0f), 0);
    prop->position()->scale = glm::vec3(7, 4, 0.1);
    prop->model()->setMesh(cubeMesh);
    prop->model()->setAlbedoTexture(concreteTexture);
    this->_sceneManager->addObject(prop);

    std::shared_ptr<LightSource> light;

    light = std::make_shared<LightSource>();
    light->type() = RECTANGLE_LIGHT_SOURCE;
    light->enabled() = false;
    light->range() = 250;
    light->position()->position = glm::vec3(10);
    light->position()->rotation = glm::vec3(glm::radians(225.0f), glm::radians(45.0f), 0);
    light->rect() = glm::vec2(20);
    this->_sceneManager->addObject(light);

    light = std::make_shared<LightSource>();
    light->type() = CONE_LIGHT_SOURCE;
    light->enabled() = false;
    light->color() = glm::vec3(1, 0, 0);
    light->range() = 50;
    light->position()->position = glm::vec3(2, 0, -2);
    light->position()->rotation = glm::vec3(glm::radians(135.0f), glm::radians(90.0f), 0);
    this->_sceneManager->addObject(light);

    light = std::make_shared<LightSource>();
    light->type() = POINT_LIGHT_SOURCE;
    light->enabled() = true;
    light->color() = glm::vec3(0, 1, 0);
    light->range() = 20;
    light->position()->position = glm::vec3(2, 2, 0);
    this->_sceneManager->addObject(light);

    light = std::make_shared<LightSource>();
    light->type() = CONE_LIGHT_SOURCE;
    light->enabled() = false;
    light->color() = glm::vec3(0, 0, 1);
    light->range() = 50;
    light->position()->position = glm::vec3(2, 0, 2);
    light->position()->rotation = glm::vec3(glm::radians(225.0f), glm::radians(90.0f), 0);
    this->_sceneManager->addObject(light);

    std::shared_ptr<Camera> camera = std::make_shared<Camera>();
    camera->position()->position = glm::vec3(2);
    camera->position()->rotation = glm::vec3(glm::radians(-135.0f), glm::radians(45.0f), 0);
    this->_sceneManager->addObject(camera);
}

void Engine::cleanup() {
    this->_renderingManager->waitIdle();

    this->_resourceManager->removeAll();
    this->_renderingManager->destroy();

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

//    TODO:        this->_debugUI->render();
//    TODO:    this->_renderer->render();

        this->_delta = glfwGetTime() - startTime;
    }
}
