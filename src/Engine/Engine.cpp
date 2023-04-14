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
#include "src/Rendering/GpuManager.hpp"
#include "src/Resources/ResourceDatabase.hpp"
#include "src/Resources/ResourceLoader.hpp"
#include "src/Resources/Readers/SceneReader.hpp"
#include "src/Objects/Camera.hpp"
#include "src/Objects/Components/PositionComponent.hpp"
#include "src/Scene/Scene.hpp"
#include "src/Scene/SceneNode.hpp"
#include "src/Scene/SceneManager.hpp"
#include "src/Debug/DebugUIRoot.hpp"

Engine::Engine()
        : _log(std::make_shared<Log>()),
          _vars(std::make_shared<VarCollection>()),
          _eventQueue(std::make_shared<EventQueue>()),
          _resourceDatabase(std::make_shared<ResourceDatabase>(this->_log,
                                                               this->_eventQueue)),
          _resourceLoader(std::make_shared<ResourceLoader>(this->_log,
                                                           this->_eventQueue)),
          _inputProcessor(std::make_shared<InputProcessor>(this->_eventQueue)),
          _window(std::make_shared<Window>(this->_vars,
                                           this->_eventQueue)),
          _gpuManager(std::make_shared<GpuManager>(this->_log,
                                                   this->_vars,
                                                   this->_eventQueue,
                                                   this->_resourceDatabase,
                                                   this->_resourceLoader,
                                                   this->_window)),
          _sceneManager(std::make_shared<SceneManager>(this->_eventQueue)),
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

    this->_resourceDatabase->tryAddDirectory("data");

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

    this->_gpuManager->init();

    this->_debugUI = std::make_shared<DebugUIRoot>(this->_log, this->_eventQueue, this->_vars, this->_resourceDatabase,
                                                   this->_resourceLoader, this->_sceneManager);

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

    std::shared_ptr<SceneReader> sceneReader = std::make_shared<SceneReader>(this->_log);

    auto sceneResource = this->_resourceDatabase->tryGetResource("data/scenes/scene1");

    if (sceneResource.has_value()) {
        auto sceneRoot = sceneReader->tryRead(this->_resourceLoader->load(sceneResource.value()));

        if (sceneRoot.has_value()) {
            std::shared_ptr<Scene> scene = Scene::empty();
            scene->root() = sceneRoot.value();

            this->_sceneManager->setScene(scene);
        }

        this->_resourceLoader->freeResource("data/scenes/scene1");
    }
}

void Engine::cleanup() {
    this->_sceneManager->setScene(nullptr);

    this->_gpuManager->destroy();

    ImGui::DestroyContext();

    this->_window->destroy();

    glfwTerminate();

    this->_resourceLoader->freeAll();
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

//        this->_debugUI->render();

        this->_delta = glfwGetTime() - startTime;
    }
}
