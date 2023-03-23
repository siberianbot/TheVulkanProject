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
class RenderingObjectsAllocator;
class VulkanObjectsAllocator;

class Renderer {
private:
    Engine *_engine;

    VkInstance _instance = VK_NULL_HANDLE;
    VkSurfaceKHR _surface = VK_NULL_HANDLE;

    std::shared_ptr<PhysicalDevice> _physicalDevice;
    std::shared_ptr<RenderingDevice> _renderingDevice;
    std::shared_ptr<VulkanObjectsAllocator> _vulkanObjectsAllocator;
    std::shared_ptr<CommandExecutor> _commandExecutor;
    std::shared_ptr<RenderingObjectsAllocator> _renderingObjectsAllocator;
    std::shared_ptr<Swapchain> _swapchain;

    struct SyncObjectsGroup {
        std::shared_ptr<FenceObject> fence;
        std::shared_ptr<SemaphoreObject> imageAvailableSemaphore;
        std::shared_ptr<SemaphoreObject> renderFinishedSemaphore;
    };

    uint32_t _currentFrameIdx = 0;
    std::array<SyncObjectsGroup *, MAX_INFLIGHT_FRAMES> _syncObjectsGroups;
    std::vector<RenderpassBase *> _renderpasses;

    void cleanupRenderpasses();

public:
    Renderer(Engine *engine);

    void init();
    void initRenderpasses();
    void cleanup();

    void render();
    void wait();

    [[nodiscard]] std::shared_ptr<RenderingObjectsAllocator> renderingObjectsAllocator() const { return this->_renderingObjectsAllocator; }
};

#endif // RENDERING_RENDERER_HPP
