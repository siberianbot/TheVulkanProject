#ifndef DEBUG_DEBUGUIRENDERSTAGE_HPP
#define DEBUG_DEBUGUIRENDERSTAGE_HPP

#include <vector>

#include "src/Rendering/Stages/RenderStage.hpp"

class GpuManager;
class LogicalDeviceProxy;

class DebugUIRenderStage : public RenderStage {
private:
    std::shared_ptr<GpuManager> _gpuManager;

    std::shared_ptr<LogicalDeviceProxy> _logicalDevice;

    vk::DescriptorPool _descriptorPool;
    vk::RenderPass _renderpass;
    std::vector<vk::Framebuffer> _framebuffers;
    vk::Rect2D _renderArea;

public:
    DebugUIRenderStage(const std::shared_ptr<GpuManager> &gpuManager);

    void init(const RenderStageInitContext &context) override;
    void destroy() override;

    void draw(uint32_t imageIdx, const vk::CommandBuffer &commandBuffer) override;
};

#endif // DEBUG_DEBUGUIRENDERSTAGE_HPP
