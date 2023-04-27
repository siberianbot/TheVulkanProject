#ifndef RENDERING_GRAPH_RENDERGRAPHEXECUTOR_HPP
#define RENDERING_GRAPH_RENDERGRAPHEXECUTOR_HPP

#include <map>
#include <memory>

#include <vulkan/vulkan.hpp>

#include "src/Rendering/Graph/RenderGraph.hpp"
#include "src/Rendering/Types/ImageView.hpp"

class GpuAllocator;
class Renderer;
class Swapchain;
class LogicalDeviceProxy;

class RenderGraphExecutor {
private:
    using FramebufferCollection = std::vector<vk::Framebuffer>;
    using ExecutionOrder = std::vector<RenderPassRef>;

    Renderer *_renderer;
    std::shared_ptr<GpuAllocator> _gpuAllocator;
    std::shared_ptr<Swapchain> _swapchain;
    std::shared_ptr<LogicalDeviceProxy> _logicalDevice;

    RenderGraph _graph;

    std::map<RenderSubgraphRef, vk::RenderPass> _renderpasses;
    std::map<RenderSubgraphRef, ExecutionOrder> _executionOrders;
    std::map<RenderSubgraphRef, FramebufferCollection> _framebuffers;

    std::map<RenderTargetRef, std::shared_ptr<ImageView>> _images;

    vk::Format processFormat(const RenderTargetFormat &format);

    vk::RenderPass processSubgraphRenderpass(const RenderSubgraph &subgraph);
    ExecutionOrder processSubgraphExecutionOrder(const RenderSubgraph &subgraph);
    FramebufferCollection processSubgraphFramebuffers(const vk::RenderPass &renderpass,
                                                      const RenderSubgraph &subgraph);

    void createFramebuffers();
    void destroyFramebuffers();

    void executeSubgraph(const RenderSubgraphRef &subgraphRef,
                         const RenderSubgraph &subgraph,
                         uint32_t imageIdx,
                         const vk::CommandBuffer &commandBuffer);

public:
    RenderGraphExecutor(Renderer *renderer,
                        const std::shared_ptr<GpuAllocator> &gpuAllocator,
                        const std::shared_ptr<Swapchain> &swapchain,
                        const std::shared_ptr<LogicalDeviceProxy> &logicalDevice,
                        const RenderGraph &graph);

    void create();
    void destroy();

    void recreateFrameBuffers();

    void execute(uint32_t imageIdx, const vk::CommandBuffer &commandBuffer);

    [[nodiscard]] const RenderGraph &getGraph() const { return this->_graph; }
};

#endif // RENDERING_GRAPH_RENDERGRAPHEXECUTOR_HPP
