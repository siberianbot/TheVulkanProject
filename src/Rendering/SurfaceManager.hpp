#ifndef RENDERING_SURFACEMANAGER_HPP
#define RENDERING_SURFACEMANAGER_HPP

#include <map>

#include <vulkan/vulkan.hpp>

struct GLFWwindow;

class Window;

class SurfaceManager {
private:
    vk::Instance _instance;

    std::map<GLFWwindow *, vk::SurfaceKHR> _surfaces;

public:
    SurfaceManager(const vk::Instance &instance);

    void destroy();

    [[nodiscard]] vk::SurfaceKHR getSurfaceFor(GLFWwindow *handle);
};

#endif // RENDERING_SURFACEMANAGER_HPP
