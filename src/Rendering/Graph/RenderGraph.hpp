#ifndef RENDERING_RENDERGRAPH_HPP
#define RENDERING_RENDERGRAPH_HPP

#include <map>
#include <memory>
#include <string>
#include <tuple>
#include <vector>

#include <vulkan/vulkan.hpp>

#include "src/Rendering/Graph/RenderSubgraph.hpp"
#include "src/Rendering/Types/ImageView.hpp"

class Log;
class GpuAllocator;
class Swapchain;
class LogicalDeviceProxy;

class RenderGraph {
private:
    struct ProcessedSubgraph {
        RenderSubgraph subgraph;
        vk::RenderPass renderPass;
        std::map<RenderTargetRef, uint32_t> attachmentsMap;
    };

    std::shared_ptr<Log> _log;
    std::shared_ptr<LogicalDeviceProxy> _logicalDevice;
    std::shared_ptr<GpuAllocator> _gpuAllocator;
    std::shared_ptr<Swapchain> _swapchain;

    std::map<RenderSubgraphRef, RenderSubgraph> _subgraphs;

    std::map<RenderTargetRef, std::shared_ptr<ImageView>> _allocatedImages;

    std::map<RenderSubgraphRef, ProcessedSubgraph> _processedSubgraphs;
    std::vector<RenderSubgraphRef> _subgraphExecutionOrder;
    std::map<RenderSubgraphRef, std::vector<vk::Framebuffer>> _framebuffers;

    vk::Format toVulkanFormat(RenderTargetFormat format);

    RenderGraph::ProcessedSubgraph processSubgraph(const RenderSubgraph &subgraph);
    std::shared_ptr<ImageView> processTarget(const RenderTarget &target);

    void createFramebufferFor(const RenderSubgraphRef &subgraphRef,
                              const ProcessedSubgraph &processedSubgraph);

public:
    RenderGraph(const std::shared_ptr<Log> &log,
                const std::shared_ptr<LogicalDeviceProxy> &logicalDevice,
                const std::shared_ptr<GpuAllocator> &gpuAllocator,
                const std::shared_ptr<Swapchain> &swapchain);

    void createGraph();
    void destroyGraph();

    void createFramebuffers();
    void destroyFrameBuffers();

    void execute(uint32_t imageIdx, const vk::CommandBuffer &commandBuffer);

    [[nodiscard]] std::map<RenderSubgraphRef, RenderSubgraph> &subgraphs() { return this->_subgraphs; }

    [[nodiscard]] const vk::RenderPass &getRenderPass(const RenderSubgraphRef &subgraphRef);
};

#endif // RENDERING_RENDERGRAPH_HPP
