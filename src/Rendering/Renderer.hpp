#ifndef RENDERING_RENDERER_HPP
#define RENDERING_RENDERER_HPP

#include <array>
#include <memory>
#include <optional>
#include <vector>

#include <vulkan/vulkan.hpp>
#include <GLFW/glfw3.h>

#include "Common.hpp"

class Engine;
class PhysicalDevice;
class RenderingDevice;
class CommandExecutor;
class Swapchain;
class FenceObject;
class SemaphoreObject;
class RenderpassBase;
class RendererAllocator;
class VulkanObjectsAllocator;

class Renderer {
private:
    struct SyncObjectsGroup {
        std::shared_ptr<FenceObject> fence;
        std::shared_ptr<SemaphoreObject> imageAvailableSemaphore;
        std::shared_ptr<SemaphoreObject> renderFinishedSemaphore;
    };

    Engine *_engine;

    uint32_t _currentFrameIdx = 0;

    VkInstance _instance = VK_NULL_HANDLE;
    VkSurfaceKHR _surface = VK_NULL_HANDLE;

    std::shared_ptr<PhysicalDevice> _physicalDevice;
    std::shared_ptr<RenderingDevice> _renderingDevice;
    std::shared_ptr<VulkanObjectsAllocator> _vulkanObjectsAllocator;
    std::shared_ptr<CommandExecutor> _commandExecutor;
    std::shared_ptr<RendererAllocator> _rendererAllocator;
    std::shared_ptr<Swapchain> _swapchain;

    std::array<SyncObjectsGroup *, MAX_INFLIGHT_FRAMES> _syncObjectsGroups;
    std::vector<RenderpassBase *> _renderpasses;

    static VKAPI_ATTR VkBool32 VKAPI_CALL debugCallback(VkDebugUtilsMessageSeverityFlagBitsEXT messageSeverity,
                                                        VkDebugUtilsMessageTypeFlagsEXT messageType,
                                                        const VkDebugUtilsMessengerCallbackDataEXT *pCallbackData,
                                                        void *pUserData);

    VkInstance createInstance();
    VkSurfaceKHR createSurface(GLFWwindow *window);

    void handleResize();

    void cleanupRenderpasses();

public:
    Renderer(Engine *engine);

    void init();
    void initRenderpasses();
    void cleanup();

    void render();
    void wait();

    [[nodiscard]] std::shared_ptr<RendererAllocator> rendererAllocator() const { return this->_rendererAllocator; }
};

#endif // RENDERING_RENDERER_HPP
