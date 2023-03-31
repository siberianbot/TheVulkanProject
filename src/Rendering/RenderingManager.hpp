#ifndef RENDERING_RENDERINGMANAGER_HPP
#define RENDERING_RENDERINGMANAGER_HPP

#include <memory>

#include <vulkan/vulkan.hpp>

class Log;
class VarCollection;
class Window;
class PhysicalDevice;
class RenderingDevice;
class CommandExecutor;
class RenderingLayoutsManager;
class RenderingObjectsAllocator;
class VulkanObjectsAllocator;
class Swapchain;

class RenderingManager {
private:
    std::shared_ptr<Log> _log;
    std::shared_ptr<VarCollection> _vars;
    std::shared_ptr<Window> _window;

    VkInstance _instance = VK_NULL_HANDLE;
    VkSurfaceKHR _surface = VK_NULL_HANDLE;

    std::shared_ptr<PhysicalDevice> _physicalDevice;
    std::shared_ptr<RenderingDevice> _renderingDevice;
    std::shared_ptr<CommandExecutor> _commandExecutor;
    std::shared_ptr<RenderingLayoutsManager> _renderingLayoutsManager;
    std::shared_ptr<RenderingObjectsAllocator> _renderingObjectsAllocator;
    std::shared_ptr<VulkanObjectsAllocator> _vulkanObjectsAllocator;
    std::shared_ptr<Swapchain> _swapchain;

    static VKAPI_ATTR VkBool32 VKAPI_CALL debugCallback(VkDebugUtilsMessageSeverityFlagBitsEXT messageSeverity,
                                                        VkDebugUtilsMessageTypeFlagsEXT messageType,
                                                        const VkDebugUtilsMessengerCallbackDataEXT *pCallbackData,
                                                        void *pUserData);

    VkInstance createInstance();
    VkSurfaceKHR createSurface();

public:
    RenderingManager(const std::shared_ptr<Log> &log,
                     const std::shared_ptr<VarCollection> &vars,
                     const std::shared_ptr<Window> &window);

    void init();
    void destroy();

    void waitIdle();

    [[nodiscard]] VkInstance instance() const { return this->_instance; }
    [[nodiscard]] const std::shared_ptr<PhysicalDevice> &physicalDevice() const { return this->_physicalDevice; }
    [[nodiscard]] const std::shared_ptr<RenderingDevice> &renderingDevice() const { return this->_renderingDevice; }
    [[nodiscard]] const std::shared_ptr<CommandExecutor> &commandExecutor() const { return this->_commandExecutor; }
    [[nodiscard]] const std::shared_ptr<RenderingLayoutsManager> &renderingLayoutsManager() const {
        return this->_renderingLayoutsManager;
    }
    [[nodiscard]] const std::shared_ptr<RenderingObjectsAllocator> &renderingObjectsAllocator() const {
        return this->_renderingObjectsAllocator;
    }
    [[nodiscard]] const std::shared_ptr<VulkanObjectsAllocator> &vulkanObjectsAllocator() const {
        return this->_vulkanObjectsAllocator;
    }
    [[nodiscard]] const std::shared_ptr<Swapchain> &swapchain() const { return this->_swapchain; }
};

#endif // RENDERING_RENDERINGMANAGER_HPP
