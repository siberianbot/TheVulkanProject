#include "Engine.hpp"

#include <cmath>

#include <GLFW/glfw3.h>
#include "subprojects/imgui-1.89.2/imgui.h"

#include "src/Engine/Log.hpp"
#include "src/Engine/VarCollection.hpp"
#include "src/Engine/Vars.hpp"
#include "src/Events/EventQueue.hpp"
#include "src/Engine/InputProcessor.hpp"
#include "src/System/Window.hpp"
#include "src/Rendering/RenderingManager.hpp"
#include "src/Rendering/Renderer.hpp"
#include "src/Resources/ResourceDatabase.hpp"
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
#include "src/Debug/DebugUIRoot.hpp"

Engine::Engine()
        : _log(std::make_shared<Log>()),
          _vars(std::make_shared<VarCollection>()),
          _eventQueue(std::make_shared<EventQueue>()),
          _resourceDatabase(std::make_shared<ResourceDatabase>(this->_log)),
          _inputProcessor(std::make_shared<InputProcessor>(this->_eventQueue)),
          _window(std::make_shared<Window>(this->_vars, this->_eventQueue)),
          _renderingManager(std::make_shared<RenderingManager>(this->_log, this->_vars, this->_window)),
          _sceneManager(std::make_shared<SceneManager>(this->_eventQueue)),
          _renderer(nullptr),
          _debugUI(nullptr) {
    //
}

void Engine::init() {
    this->_vars->set(WINDOW_TITLE_VAR, "TheVulkanProject");
    this->_vars->set(WINDOW_WIDTH_VAR, 1280);
    this->_vars->set(WINDOW_HEIGHT_VAR, 720);
    this->_vars->set(RENDERING_SCENE_STAGE_LIGHT_COUNT, 128);
    this->_vars->set(RENDERING_SCENE_STAGE_SHADOW_MAP_COUNT, 32);
    this->_vars->set(RENDERING_SCENE_STAGE_SHADOW_MAP_SIZE, 1024);
    this->_vars->set(RESOURCES_DEFAULT_TEXTURE, "textures/default");

    if (glfwInit() != GLFW_TRUE) {
        throw std::runtime_error("Failed to initilalize GLFW");
    }

    // TODO: put Dear ImGui somewhere
    ImGui::CreateContext();
    ImGui::StyleColorsDark();

    this->_window->create();

    this->_eventQueue->addHandler([this](const Event &event) {
        if (event.type != CLOSE_REQUESTED_EVENT) {
            return;
        }

        this->_work = false;
    });

    this->_renderingManager->init();
//
//    this->_resourceManager = std::make_shared<ResourceManager>(this->_log,
//                                                               this->_renderingManager->renderingObjectsAllocator());
    this->_resourceDatabase->tryAddDirectory("data");

    this->_debugUI = std::make_shared<DebugUIRoot>(this->_log, this->_eventQueue, this->_vars, this->_resourceDatabase,
                                                   this->_sceneManager);

    this->_renderer = std::make_shared<Renderer>(this->_vars, this->_eventQueue, this->_renderingManager,
                                                 this->_sceneManager);
    this->_renderer->init();

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
        double dx = this->_x - x;
        double dy = this->_y - y;

        this->_x = x;
        this->_y = y;

        if (this->_state != Focused ||
            this->_sceneManager->currentCamera().expired()) {
            return;
        }

        std::shared_ptr<Camera> camera = this->_sceneManager->currentCamera().lock();

        if (camera == nullptr) {
            return;
        }

        const float sensitivity = 0.0005f;
        const auto normalize = [](float value) -> float {
            if (value < 0) {
                return value + 2 * M_PI;
            } else if (value > 2 * M_PI) {
                return value - 2 * M_PI;
            }

            return value;
        };

        camera->position()->rotation().x = normalize(camera->position()->rotation().x - sensitivity * (float) dx);
        camera->position()->rotation().y = normalize(camera->position()->rotation().y + sensitivity * (float) dy);
    });

//    std::shared_ptr<MeshResource> cubeMesh = this->_resourceManager->tryGetResource<MeshResource>(
//            "cube", MESH_RESOURCE).value().lock();
//    cubeMesh->load();
//
//    std::shared_ptr<MeshResource> skyboxMesh = this->_resourceManager->tryGetResource<MeshResource>(
//            "skybox", MESH_RESOURCE).value().lock();
//    skyboxMesh->load();
//
//    std::shared_ptr<MeshResource> suzanneMesh = this->_resourceManager->tryGetResource<MeshResource>(
//            "suzanne", MESH_RESOURCE).value().lock();
//    suzanneMesh->load();
//
//    std::shared_ptr<MeshResource> vikingRoomMesh = this->_resourceManager->tryGetResource<MeshResource>(
//            "viking_room", MESH_RESOURCE).value().lock();
//    vikingRoomMesh->load();
//
//    std::shared_ptr<ImageResource> defaultTexture = this->_resourceManager->tryGetResource<ImageResource>(
//            "textures/default", IMAGE_RESOURCE).value().lock();
//    defaultTexture->load();
//
//    std::shared_ptr<ImageResource> concreteTexture = this->_resourceManager->tryGetResource<ImageResource>(
//            "concrete", IMAGE_RESOURCE).value().lock();
//    concreteTexture->load();
//
//    std::shared_ptr<ImageResource> cubeTexture = this->_resourceManager->tryGetResource<ImageResource>(
//            "cube_texture", IMAGE_RESOURCE).value().lock();
//    cubeTexture->load();
//
//    std::shared_ptr<ImageResource> cubeSpecularTexture = this->_resourceManager->tryGetResource<ImageResource>(
//            "cube_texture_specular", IMAGE_RESOURCE).value().lock();
//    cubeSpecularTexture->load();
//
//    std::shared_ptr<ImageResource> vikingRoomTexture = this->_resourceManager->tryGetResource<ImageResource>(
//            "viking_room_texture", IMAGE_RESOURCE).value().lock();
//    vikingRoomTexture->load();
//
//    std::shared_ptr<CubeImageResource> skyboxTexture = this->_resourceManager->tryGetResource<CubeImageResource>(
//            "skybox_texture", CUBE_IMAGE_RESOURCE).value().lock();
//    skyboxTexture->load();
//
//
//    this->_sceneManager->setScene(Scene::empty());
//
//    std::shared_ptr<World> world = std::make_shared<World>();
//    world->skybox()->setMesh(skyboxMesh);
//    world->skybox()->setTexture(skyboxTexture);
//    this->_sceneManager->currentScene()->root()->object() = world;
//
//    std::shared_ptr<Prop> prop;
//
//    prop = std::make_shared<Prop>();
//    prop->position()->position() = glm::vec3(0, 0, 2);
//    prop->model()->setMesh(vikingRoomMesh);
//    prop->model()->setAlbedoTexture(vikingRoomTexture);
//    this->_sceneManager->addObject(prop);
//
//    prop = std::make_shared<Prop>();
//    prop->position()->position() = glm::vec3(0);
//    prop->position()->scale() = glm::vec3(0.5f);
//    prop->model()->setMesh(cubeMesh);
//    prop->model()->setAlbedoTexture(cubeTexture);
//    prop->model()->setSpecularTexture(cubeSpecularTexture);
//    this->_sceneManager->addObject(prop);
//
//    prop = std::make_shared<Prop>();
//    prop->position()->position() = glm::vec3(0, 0, -2);
//    prop->position()->scale() = glm::vec3(0.5f);
//    prop->model()->setMesh(suzanneMesh);
//    prop->model()->setAlbedoTexture(concreteTexture);
//    prop->model()->setSpecularTexture(defaultTexture);
//    this->_sceneManager->addObject(prop);
//
//    // down
//    prop = std::make_shared<Prop>();
//    prop->position()->position() = glm::vec3(0, -4, 0);
//    prop->position()->rotation() = glm::vec3(glm::radians(270.0f), 0, 0);
//    prop->position()->scale() = glm::vec3(7, 9, 0.1);
//    prop->model()->setMesh(cubeMesh);
//    prop->model()->setAlbedoTexture(concreteTexture);
//    this->_sceneManager->addObject(prop);
//
//    // front
//    prop = std::make_shared<Prop>();
//    prop->position()->position() = glm::vec3(7, 0, 0);
//    prop->position()->rotation() = glm::vec3(0, glm::radians(270.0f), 0);
//    prop->position()->scale() = glm::vec3(9, 4, 0.1);
//    prop->model()->setMesh(cubeMesh);
//    prop->model()->setAlbedoTexture(concreteTexture);
//    this->_sceneManager->addObject(prop);
//
//    // back
//    prop = std::make_shared<Prop>();
//    prop->position()->position() = glm::vec3(-7, 0, 0);
//    prop->position()->rotation() = glm::vec3(0, glm::radians(90.0f), 0);
//    prop->position()->scale() = glm::vec3(9, 4, 0.1);
//    prop->model()->setMesh(cubeMesh);
//    prop->model()->setAlbedoTexture(concreteTexture);
//    this->_sceneManager->addObject(prop);
//
//    // left
//    prop = std::make_shared<Prop>();
//    prop->position()->position() = glm::vec3(0, 0, -9);
//    prop->position()->rotation() = glm::vec3(0);
//    prop->position()->scale() = glm::vec3(7, 4, 0.1);
//    prop->model()->setMesh(cubeMesh);
//    prop->model()->setAlbedoTexture(concreteTexture);
//    this->_sceneManager->addObject(prop);
//
//    // right
//    prop = std::make_shared<Prop>();
//    prop->position()->position() = glm::vec3(0, 0, 9);
//    prop->position()->rotation() = glm::vec3(0, glm::radians(180.0f), 0);
//    prop->position()->scale() = glm::vec3(7, 4, 0.1);
//    prop->model()->setMesh(cubeMesh);
//    prop->model()->setAlbedoTexture(concreteTexture);
//    this->_sceneManager->addObject(prop);
//
//    std::shared_ptr<LightSource> light;
//
//    light = std::make_shared<LightSource>();
//    light->type() = RECTANGLE_LIGHT_SOURCE;
//    light->enabled() = false;
//    light->range() = 250;
//    light->position()->position() = glm::vec3(10);
//    light->position()->rotation() = glm::vec3(glm::radians(225.0f), glm::radians(45.0f), 0);
//    light->rect() = glm::vec2(20);
//    this->_sceneManager->addObject(light);
//
//    light = std::make_shared<LightSource>();
//    light->type() = CONE_LIGHT_SOURCE;
//    light->enabled() = false;
//    light->color() = glm::vec3(1, 0, 0);
//    light->range() = 50;
//    light->position()->position() = glm::vec3(2, 0, -2);
//    light->position()->rotation() = glm::vec3(glm::radians(135.0f), glm::radians(90.0f), 0);
//    this->_sceneManager->addObject(light);
//
//    light = std::make_shared<LightSource>();
//    light->type() = POINT_LIGHT_SOURCE;
//    light->enabled() = true;
//    light->color() = glm::vec3(0, 1, 0);
//    light->range() = 20;
//    light->position()->position() = glm::vec3(2, 2, 0);
//    this->_sceneManager->addObject(light);
//
//    light = std::make_shared<LightSource>();
//    light->type() = CONE_LIGHT_SOURCE;
//    light->enabled() = false;
//    light->color() = glm::vec3(0, 0, 1);
//    light->range() = 50;
//    light->position()->position() = glm::vec3(2, 0, 2);
//    light->position()->rotation() = glm::vec3(glm::radians(225.0f), glm::radians(90.0f), 0);
//    this->_sceneManager->addObject(light);
//
//    std::shared_ptr<Camera> camera = std::make_shared<Camera>();
//    camera->position()->position() = glm::vec3(2, 0, 2);
//    camera->position()->rotation() = glm::vec3(0, glm::radians(180.0f), 0);
//    this->_sceneManager->addObject(camera);
//    this->_sceneManager->currentCamera() = camera;
}

void Engine::cleanup() {
    this->_renderingManager->waitIdle();

    this->_sceneManager->setScene(nullptr);

    this->_renderer->destroy();

    this->_renderingManager->destroy();

    ImGui::DestroyContext();

    this->_window->destroy();

    glfwTerminate();

    this->_resourceDatabase->clear();
}

void Engine::run() {
    this->_work = true;

    while (this->_work) {
        double startTime = glfwGetTime();

        glfwPollEvents();

        this->_eventQueue->process();

        if (!this->_sceneManager->currentCamera().expired()) {
            std::shared_ptr<Camera> camera = this->_sceneManager->currentCamera().lock();

            auto forward = camera->forward();
            auto side = camera->side();

            if (this->_moveForward) {
                camera->position()->position() += 5 * this->_delta * forward;
            }

            if (this->_moveBackward) {
                camera->position()->position() -= 5 * this->_delta * forward;
            }

            if (this->_strafeLeft) {
                camera->position()->position() -= 5 * this->_delta * side;
            }

            if (this->_strafeRight) {
                camera->position()->position() += 5 * this->_delta * side;
            }
        }

        this->_debugUI->render();
        this->_renderer->render();

        this->_delta = glfwGetTime() - startTime;
    }
}
