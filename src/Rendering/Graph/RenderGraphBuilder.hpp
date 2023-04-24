#ifndef RENDERING_GRAPH_RENDERGRAPHBUILDER_HPP
#define RENDERING_GRAPH_RENDERGRAPHBUILDER_HPP

#include <map>
#include <memory>

#include "src/Rendering/Graph/RenderGraph.hpp"
#include "src/Rendering/Graph/RenderSubgraph.hpp"
#include "src/Rendering/Types/ImageRequirements.hpp"

class Swapchain;
class RenderGraph;

class RenderGraphBuilder {
private:
    std::shared_ptr<Log> _log;
    std::shared_ptr<LogicalDeviceProxy> _logicalDevice;
    std::shared_ptr<GpuAllocator> _gpuAllocator;
    std::shared_ptr<Swapchain> _swapchain;

    std::map<RenderSubgraphRef, RenderSubgraph> _subgraphs;

    vk::Format toVulkanFormat(RenderTargetFormat format);

    RenderGraph::SubgraphInfo processSubgraph(const RenderSubgraph &subgraph);
    ImageRequirements processTarget(const RenderTarget &target);
    std::vector<RenderSubgraphRef> processExecutionOrder();

public:
    RenderGraphBuilder(const std::shared_ptr<Log> &log,
                       const std::shared_ptr<LogicalDeviceProxy> &logicalDevice,
                       const std::shared_ptr<GpuAllocator> &gpuAllocator,
                       const std::shared_ptr<Swapchain> &swapchain);

    RenderGraphBuilder &addSubgraph(const RenderSubgraphRef &ref, const RenderSubgraph &subgraph);

    std::shared_ptr<RenderGraph> build();
};

#endif // RENDERING_GRAPH_RENDERGRAPHBUILDER_HPP
