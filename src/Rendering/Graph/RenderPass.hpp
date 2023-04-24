#ifndef RENDERING_GRAPH_RENDERPASS_HPP
#define RENDERING_GRAPH_RENDERPASS_HPP

#include <functional>
#include <vector>

#include <vulkan/vulkan.hpp>

#include "src/Rendering/Graph/RenderTargetRef.hpp"

using RenderPassAction = std::function<void(const vk::CommandBuffer &commandBuffer)>;

struct RenderPassInput {
    RenderTargetRef targetRef;
};

struct RenderPass {
    std::vector<RenderPassInput> inputs;
    RenderPassAction action;
};

#endif // RENDERING_GRAPH_RENDERPASS_HPP
