#ifndef RENDERING_GRAPH_RENDERPASS_HPP
#define RENDERING_GRAPH_RENDERPASS_HPP

#include <string>
#include <vector>

#include <vulkan/vulkan.hpp>

#include "src/Rendering/Graph/RenderTarget.hpp"

using RenderPassRef = std::string;

struct RenderPassInput {
    RenderTargetRef targetRef;
};

struct RenderPassOutput {
    RenderTargetRef targetRef;
    vk::AttachmentLoadOp loadOp;
    vk::AttachmentStoreOp storeOp;
    vk::ImageLayout initialLayout;
    vk::ImageLayout finalLayout;
};

struct RenderPass {
    std::vector<RenderPassInput> inputs;
    std::vector<RenderPassOutput> outputs;
};

#endif // RENDERING_GRAPH_RENDERPASS_HPP
