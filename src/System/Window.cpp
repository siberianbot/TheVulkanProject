#include "Window.hpp"

#include <stdexcept>

#include <GLFW/glfw3.h>
#include <imgui.h>
#include <imgui_impl_glfw.h>

#include "src/Engine/EngineVars.hpp"
#include "src/Events/EventQueue.hpp"

void Window::mouseButtonCallback(GLFWwindow *window, int button, int action, int mods) {
    Window *that = reinterpret_cast<Window *>(glfwGetWindowUserPointer(window));

    if (ImGui::GetIO().WantCaptureMouse) {
        return;
    }

    EventType type;
    switch (action) {
        case GLFW_PRESS:
            type = MOUSE_PRESS_INPUT_EVENT;
            break;

        case GLFW_RELEASE:
            type = MOUSE_RELEASE_INPUT_EVENT;
            break;

        default:
            throw std::runtime_error("Not supported mouse button action");
    }

    Event event = {
            .type = type,
            .value = InputData{
                    .mouseButton = button
            }
    };

    that->_eventQueue->pushEvent(event);
}

void Window::framebufferResizeCallback(GLFWwindow *window, int width, int height) {
    Window *that = reinterpret_cast<Window *>(glfwGetWindowUserPointer(window));

    that->_width = width;
    that->_height = height;

    Event event = {
            .type = RESIZE_WINDOW_EVENT,
            .value = WindowData{
                    .width = static_cast<uint32_t>(width),
                    .height = static_cast<uint32_t>(height)
            }
    };

    that->_eventQueue->pushEvent(event);
}

void Window::keyCallback(GLFWwindow *window, int key, int scancode, int action, int mods) {
    Window *that = reinterpret_cast<Window *>(glfwGetWindowUserPointer(window));

    if (ImGui::GetIO().WantCaptureKeyboard) {
        return;
    }

    EventType type;
    switch (action) {
        case GLFW_PRESS:
            type = KEYBOARD_PRESS_INPUT_EVENT;
            break;

        case GLFW_RELEASE:
            type = KEYBOARD_RELEASE_INPUT_EVENT;
            break;

        case GLFW_REPEAT:
            return; // not supported by design

        default:
            throw std::runtime_error("Not supported keyboard action");
    }

    Event event = {
            .type = type,
            .value = InputData{
                    .keyboardKey = key
            }
    };

    that->_eventQueue->pushEvent(event);
}

void Window::cursorCallback(GLFWwindow *window, double x, double y) {
    Window *that = reinterpret_cast<Window *>(glfwGetWindowUserPointer(window));

    Event event = {
            .type = CURSOR_MOVE_INPUT_EVENT,
            .value = InputData{
                    .cursorHorizontal = x,
                    .cursorVertical = y
            }
    };

    that->_eventQueue->pushEvent(event);
}

void Window::closeCallback(GLFWwindow *window) {
    Window *that = reinterpret_cast<Window *>(glfwGetWindowUserPointer(window));

    Event event = {
            .type = CLOSE_REQUESTED_EVENT
    };

    that->_eventQueue->pushEvent(event);
}

Window::Window(const std::shared_ptr<EngineVars> &engineVars, const std::shared_ptr<EventQueue> &eventQueue)
        : _engineVars(engineVars),
          _eventQueue(eventQueue) {
    //
}

void Window::create() {
    if (this->_handle != nullptr) {
        return;
    }

    glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);

    this->_width = this->_engineVars->getOrDefault(WINDOW_WIDTH_VAR, 1280)->intValue;
    this->_height = this->_engineVars->getOrDefault(WINDOW_HEIGHT_VAR, 720)->intValue;
    char *title = this->_engineVars->getOrDefault(WINDOW_TITLE_VAR, "Unknown")->stringValue;

    this->_handle = glfwCreateWindow(this->_width, this->_height, title, nullptr, nullptr);

    if (this->_handle == nullptr) {
        throw std::runtime_error("Failed to create window");
    }

    glfwSetWindowUserPointer(this->_handle, this);
    glfwSetFramebufferSizeCallback(this->_handle, framebufferResizeCallback);
    glfwSetKeyCallback(this->_handle, keyCallback);
    glfwSetMouseButtonCallback(this->_handle, mouseButtonCallback);
    glfwSetCursorPosCallback(this->_handle, cursorCallback);
    glfwSetWindowCloseCallback(this->_handle, closeCallback);

    ImGui_ImplGlfw_InitForVulkan(this->_handle, true);

    glfwGetCursorPos(this->_handle, &this->_cursorX, &this->_cursorY);
}

void Window::destroy() {
    if (this->_handle == nullptr) {
        return;
    }

    glfwDestroyWindow(this->_handle);

    this->_handle = nullptr;
}

void Window::hideCursor() {
    glfwSetInputMode(this->_handle, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
}

void Window::showCursor() {
    glfwSetInputMode(this->_handle, GLFW_CURSOR, GLFW_CURSOR_NORMAL);
}
