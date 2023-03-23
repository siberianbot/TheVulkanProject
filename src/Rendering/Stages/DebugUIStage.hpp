#ifndef RENDERING_STAGES_DEBUGUISTAGE_HPP
#define RENDERING_STAGES_DEBUGUISTAGE_HPP

#include <memory>

#include "src/Rendering/Stages/StageBase.hpp"

class EventQueue;
class PhysicalDevice;
class RenderingDevice;
class CommandExecutor;
class Swapchain;
class VulkanObjectsAllocator;
class DebugUIRenderpass;

class DebugUIStage : public StageBase {
private:
    std::shared_ptr<EventQueue> _eventQueue;
    std::shared_ptr<PhysicalDevice> _physicalDevice;
    std::shared_ptr<RenderingDevice> _renderingDevice;
    std::shared_ptr<VulkanObjectsAllocator> _vulkanObjectsAllocator;
    std::shared_ptr<Swapchain> _swapchain;
    std::shared_ptr<CommandExecutor> _commandExecutor;
    VkInstance _instance;

    VkDescriptorPool _descriptorPool;
    std::shared_ptr<DebugUIRenderpass> _debugUIRenderpass;

public:
    DebugUIStage(const std::shared_ptr<EventQueue> &eventQueue,
                 const std::shared_ptr<PhysicalDevice> &physicalDevice,
                 const std::shared_ptr<RenderingDevice> &renderingDevice,
                 const std::shared_ptr<VulkanObjectsAllocator> &vulkanObjectsAllocator,
                 const std::shared_ptr<Swapchain> &swapchain,
                 const std::shared_ptr<CommandExecutor> &commandExecutor,
                 VkInstance instance);
    ~DebugUIStage() override = default;

    void init() override;
    void destroy() override;

    void record(VkCommandBuffer commandBuffer, uint32_t frameIdx, uint32_t imageIdx) override;
};

#endif // RENDERING_STAGES_DEBUGUISTAGE_HPP
