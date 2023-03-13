#include "Engine.hpp"

#include <imgui.h>
#include <imgui_impl_glfw.h>

#include "Constants.hpp"
#include "EngineVars.hpp"
#include "src/Events/EventQueue.hpp"
#include "src/Resources/Mesh.hpp"
#include "src/Resources/Texture.hpp"
#include "src/Resources/ResourceManager.hpp"
#include "src/Objects/Light.hpp"
#include "src/Objects/Object.hpp"
#include "src/Scene/Scene.hpp"
#include "src/Debug/DebugUI.hpp"
#include "src/Resources/Meshes.hpp"
#include "src/Objects/Skybox.hpp"
#include "src/Scene/SceneManager.hpp"

#include <glm/vec3.hpp>

Engine::Engine()
        : renderer(this),
          _eventQueue(new EventQueue()),
          _engineVars(EngineVars::defaults()),
          _resourceManager(new ResourceManager()),
          _sceneManager(new SceneManager(this->_eventQueue)) {
    //
}

void Engine::init() {
    this->_resourceManager->addDataDir("data");

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

    this->renderer.init();

    this->input.addReleaseHandler(GLFW_KEY_ESCAPE, [this](float delta) {
        if (this->_state != Focused) {
            return;
        }

        this->_state = NotFocused;
        glfwSetInputMode(this->_window, GLFW_CURSOR, GLFW_CURSOR_NORMAL);
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

    this->_meshes.push_back(this->renderer.getRenderingResourcesManager()->loadMesh(CUBE_MESH.size(),
                                                                                    CUBE_MESH.data()));

    Mesh *suzanneMesh = this->_resourceManager->openMesh("suzanne");
    this->_meshes.push_back(this->renderer.getRenderingResourcesManager()->loadMesh(suzanneMesh));
    delete suzanneMesh;

    Mesh *vikingRoomMesh = this->_resourceManager->openMesh("viking_room");
    this->_meshes.push_back(this->renderer.getRenderingResourcesManager()->loadMesh(vikingRoomMesh));
    delete vikingRoomMesh;

    Texture *defaultTexture = this->_resourceManager->openTexture("default_texture");
    this->_defaultTextureResource = this->renderer.getRenderingResourcesManager()->loadTexture(defaultTexture);
    delete defaultTexture;

    Texture *concreteTexture = this->_resourceManager->openTexture("concrete");
    this->_textures.push_back(this->renderer.getRenderingResourcesManager()->loadTexture(concreteTexture));
    delete concreteTexture;

    Texture *cubeTexture = this->_resourceManager->openTexture("cube");
    this->_textures.push_back(this->renderer.getRenderingResourcesManager()->loadTexture(cubeTexture));
    delete cubeTexture;

    Texture *cubeSpecularTexture = this->_resourceManager->openTexture("cube_specular");
    this->_textures.push_back(this->renderer.getRenderingResourcesManager()->loadTexture(cubeSpecularTexture));
    delete cubeSpecularTexture;

    Texture *vikingRoomTexture = this->_resourceManager->openTexture("viking_room_texture");
    this->_textures.push_back(this->renderer.getRenderingResourcesManager()->loadTexture(vikingRoomTexture));
    delete vikingRoomTexture;

    this->_skyboxMeshResource = this->renderer.getRenderingResourcesManager()->loadMesh(SKYBOX_MESH.size(),
                                                                                        SKYBOX_MESH.data());
    Texture *skyboxRightTexture = this->_resourceManager->openTexture("skybox_right");
    Texture *skyboxLeftTexture = this->_resourceManager->openTexture("skybox_left");
    Texture *skyboxUpTexture = this->_resourceManager->openTexture("skybox_up");
    Texture *skyboxDownTexture = this->_resourceManager->openTexture("skybox_down");
    Texture *skyboxFrontTexture = this->_resourceManager->openTexture("skybox_front");
    Texture *skyboxBackTexture = this->_resourceManager->openTexture("skybox_back");
    this->_skyboxTextureResource = this->renderer.getRenderingResourcesManager()->loadTextureCube(
            {
                    skyboxFrontTexture,
                    skyboxBackTexture,
                    skyboxUpTexture,
                    skyboxDownTexture,
                    skyboxRightTexture,
                    skyboxLeftTexture,
            });
    delete skyboxFrontTexture;
    delete skyboxBackTexture;
    delete skyboxUpTexture;
    delete skyboxDownTexture;
    delete skyboxRightTexture;
    delete skyboxLeftTexture;

    Scene *scene = new Scene(this, new Skybox(&this->_skyboxMeshResource, &this->_skyboxTextureResource));

    Object *object;

    object = new Object();
    object->position() = glm::vec3(0, 0, 2);
    object->mesh() = &this->_meshes[2];
    object->albedoTexture() = &this->_textures[3];
    scene->addObject(object);

    object = new Object();
    object->position() = glm::vec3(0);
    object->scale() = glm::vec3(0.5f);
    object->mesh() = &this->_meshes[0];
    object->albedoTexture() = &this->_textures[1];
    object->specTexture() = &this->_textures[2];
    scene->addObject(object);

    object = new Object();
    object->position() = glm::vec3(0, 0, -2);
    object->scale() = glm::vec3(0.5f);
    object->mesh() = &this->_meshes[1];
    object->albedoTexture() = &this->_textures[0];
    scene->addObject(object);

    // down
    object = new Object();
    object->position() = glm::vec3(0, -4, 0);
    object->rotation() = glm::vec3(glm::radians(270.0f), 0, 0);
    object->scale() = glm::vec3(7, 9, 0.1);
    object->mesh() = &this->_meshes[0];
    object->albedoTexture() = &this->_textures[0];
    scene->addObject(object);

    // front
    object = new Object();
    object->position() = glm::vec3(7, 0, 0);
    object->rotation() = glm::vec3(0, glm::radians(270.0f), 0);
    object->scale() = glm::vec3(9, 4, 0.1);
    object->mesh() = &this->_meshes[0];
    object->albedoTexture() = &this->_textures[0];
    scene->addObject(object);

    // back
    object = new Object();
    object->position() = glm::vec3(-7, 0, 0);
    object->rotation() = glm::vec3(0, glm::radians(90.0f), 0);
    object->scale() = glm::vec3(9, 4, 0.1);
    object->mesh() = &this->_meshes[0];
    object->albedoTexture() = &this->_textures[0];
    scene->addObject(object);

    // left
    object = new Object();
    object->position() = glm::vec3(0, 0, -9);
    object->rotation() = glm::vec3(0);
    object->scale() = glm::vec3(7, 4, 0.1);
    object->mesh() = &this->_meshes[0];
    object->albedoTexture() = &this->_textures[0];
    scene->addObject(object);

    // right
    object = new Object();
    object->position() = glm::vec3(0, 0, 9);
    object->rotation() = glm::vec3(0, glm::radians(180.0f), 0);
    object->scale() = glm::vec3(7, 4, 0.1);
    object->mesh() = &this->_meshes[0];
    object->albedoTexture() = &this->_textures[0];
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

    this->_camera.position() = glm::vec3(2, 2, 2);
    this->_camera.yaw() = glm::radians(-135.0f);
    this->_camera.pitch() = glm::radians(45.0f);

    this->_sceneManager->switchScene(scene);
}

void Engine::cleanup() {
    this->renderer.wait();

    for (TextureRenderingResource &texture: this->_textures) {
        this->renderer.getRenderingResourcesManager()->freeTexture(texture);
    }

    for (MeshRenderingResource &mesh: this->_meshes) {
        this->renderer.getRenderingResourcesManager()->freeMesh(mesh);
    }

    this->renderer.getRenderingResourcesManager()->freeTexture(this->_defaultTextureResource);
    this->renderer.getRenderingResourcesManager()->freeTexture(this->_skyboxTextureResource);
    this->renderer.getRenderingResourcesManager()->freeMesh(this->_skyboxMeshResource);

    this->renderer.cleanup();

    ImGui::DestroyContext();

    if (this->_window) {
        glfwDestroyWindow(this->_window);
    }

    glfwTerminate();

    delete this->_sceneManager;
    delete this->_resourceManager;
    delete this->_engineVars;
    delete this->_eventQueue;
}

void Engine::run() {
    while (!glfwWindowShouldClose(this->_window)) {
        double startTime = glfwGetTime();

        this->_eventQueue->process();

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
