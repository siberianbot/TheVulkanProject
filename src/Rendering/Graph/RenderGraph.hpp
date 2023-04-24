#ifndef RENDERING_RENDERGRAPH_HPP
#define RENDERING_RENDERGRAPH_HPP

#include <map>
#include <memory>
#include <string>
#include <tuple>
#include <vector>

#include <vulkan/vulkan.hpp>

#include "src/Rendering/Graph/RenderSubgraph.hpp"
#include "src/Rendering/Types/ImageRequirements.hpp"
#include "src/Rendering/Types/ImageView.hpp"

class Log;
class GpuAllocator;
class Swapchain;
class LogicalDeviceProxy;

class RenderGraph {
public:
    struct SubgraphInfo {
        RenderSubgraph subgraph;
        std::map<RenderTargetRef, uint32_t> attachmentsMap;
        std::map<RenderTargetRef, ImageRequirements> imageRequirements;
        vk::RenderPassCreateInfo renderPassCreateInfo;
        std::vector<RenderPassRef> executionOrder;
        std::vector<vk::AttachmentReference *> depthReferences;
        std::vector<vk::ClearValue> clearValues;
    };

private:
    std::shared_ptr<Log> _log;
    std::shared_ptr<LogicalDeviceProxy> _logicalDevice;
    std::shared_ptr<GpuAllocator> _gpuAllocator;
    std::shared_ptr<Swapchain> _swapchain;

    std::map<RenderSubgraphRef, SubgraphInfo> _subgraphs;
    std::vector<RenderSubgraphRef> _executionOrder;

    bool _graphBuilt;
    bool _framebuffersBuilt;

    std::map<RenderTargetRef, std::shared_ptr<ImageView>> _allocatedImages;
    std::map<RenderSubgraphRef, vk::RenderPass> _renderpasses;
    std::map<RenderSubgraphRef, std::vector<vk::Framebuffer>> _framebuffers;

    std::vector<vk::Framebuffer> createFramebuffersFor(const RenderSubgraphRef &subgraphRef,
                                                       const RenderGraph::SubgraphInfo &subgraph);

public:
    RenderGraph(const std::shared_ptr<Log> &log,
                const std::shared_ptr<LogicalDeviceProxy> &logicalDevice,
                const std::shared_ptr<GpuAllocator> &gpuAllocator,
                const std::shared_ptr<Swapchain> &swapchain,
                const std::map<RenderSubgraphRef, SubgraphInfo> &subgraphs,
                const std::vector<RenderSubgraphRef> &executionOrder);
    ~RenderGraph();

    void createRenderpasses();
    void destroyRenderpasses();

    void createFramebuffers();
    void destroyFrameBuffers();

    void invalidateFramebuffers();

    void execute(uint32_t imageIdx, const vk::CommandBuffer &commandBuffer);

    [[nodiscard]] bool graphBuilt() const { return this->_graphBuilt; }

    [[nodiscard]] bool framebuffersBuilt() const { return this->_framebuffersBuilt; }

    [[nodiscard]] const vk::RenderPass &getRenderPass(const RenderSubgraphRef &subgraphRef);
};

#endif // RENDERING_RENDERGRAPH_HPP
