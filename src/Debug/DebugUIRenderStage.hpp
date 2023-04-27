#ifndef DEBUG_DEBUGUIRENDERSTAGE_HPP
#define DEBUG_DEBUGUIRENDERSTAGE_HPP

#include <memory>
#include <vector>

#include "src/Rendering/Graph/RenderStage.hpp"

class CommandManager;
class GpuManager;
class LogicalDeviceProxy;
class PhysicalDeviceProxy;
class DebugUIRoot;

class DebugUIRenderStage : public RenderStage, public std::enable_shared_from_this<DebugUIRenderStage> {
private:
    std::shared_ptr<GpuManager> _gpuManager;
    std::shared_ptr<DebugUIRoot> _debugUIRoot;

    std::shared_ptr<CommandManager> _commandManager;
    std::shared_ptr<LogicalDeviceProxy> _logicalDevice;
    std::shared_ptr<PhysicalDeviceProxy> _physicalDevice;

    vk::DescriptorPool _descriptorPool;

public:
    DebugUIRenderStage(const std::shared_ptr<GpuManager> &gpuManager,
                       const std::shared_ptr<DebugUIRoot> &debugUIRoot);
    ~DebugUIRenderStage() override = default;

    void init() override;
    void destroy() override;

    void onGraphCreate(const std::shared_ptr<Swapchain> swapchain,
                       const vk::RenderPass &renderPass) override;
    void onGraphDestroy() override;

    void onPassExecute(const RenderPassRef &passRef, const vk::CommandBuffer &commandBuffer) override;

    RenderSubgraph asSubgraph() override;
};

#endif // DEBUG_DEBUGUIRENDERSTAGE_HPP
