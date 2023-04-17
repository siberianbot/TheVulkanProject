#include "SurfaceManager.hpp"

#include <GLFW/glfw3.h>

#include "src/Engine/EngineError.hpp"

SurfaceManager::SurfaceManager(const vk::Instance &instance)
        : _instance(instance) {
    //
}

void SurfaceManager::destroy() {
    for (const auto &[handle, surface]: this->_surfaces) {
        this->_instance.destroy(surface);
    }

    this->_surfaces.clear();
}

vk::SurfaceKHR SurfaceManager::getSurfaceFor(GLFWwindow *handle) {
    auto it = this->_surfaces.find(handle);

    if (it != this->_surfaces.end()) {
        return it->second;
    }

    VkSurfaceKHR surface;
    if (glfwCreateWindowSurface(this->_instance, handle, nullptr, &surface) != VK_SUCCESS) {
        throw EngineError("Failed to create surface from window handle");
    }

    this->_surfaces[handle] = surface;

    return surface;
}
