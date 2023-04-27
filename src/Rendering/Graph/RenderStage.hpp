#ifndef RENDERING_GRAPH_RENDERSTAGE_HPP
#define RENDERING_GRAPH_RENDERSTAGE_HPP

#include <memory>

#include <vulkan/vulkan.hpp>

#include "src/Rendering/Graph/RenderGraph.hpp"

class Swapchain;

class RenderStage {
public:
    virtual ~RenderStage() = default;

    virtual void init() = 0;
    virtual void destroy() = 0;

    virtual void onGraphCreate(const std::shared_ptr<Swapchain> swapchain,
                               const vk::RenderPass &renderPass) = 0;
    virtual void onGraphDestroy() = 0;

    virtual void onPassExecute(const RenderPassRef &passRef,
                               const vk::CommandBuffer &commandBuffer) = 0;

    virtual RenderSubgraph asSubgraph() = 0;
};

#endif // RENDERING_GRAPH_RENDERSTAGE_HPP
