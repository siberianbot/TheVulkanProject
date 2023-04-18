#ifndef DEBUG_DEBUGUIRENDERSTAGE_HPP
#define DEBUG_DEBUGUIRENDERSTAGE_HPP

#include <memory>
#include <vector>

#include <vulkan/vulkan.hpp>

class GpuManager;
class Swapchain;

class DebugUIRenderStage {
private:
    std::shared_ptr<GpuManager> _gpuManager;
    std::shared_ptr<Swapchain> _swapchain;

    vk::RenderPass _renderpass;
    std::vector<vk::Framebuffer> _framebuffers;

public:
    DebugUIRenderStage(const std::shared_ptr<GpuManager> &gpuManager,
                       const std::shared_ptr<Swapchain> &swapchain);

    void init();
    void destroy();

    void draw(uint32_t imageIdx, const vk::CommandBuffer &commandBuffer);
};

#endif // DEBUG_DEBUGUIRENDERSTAGE_HPP
