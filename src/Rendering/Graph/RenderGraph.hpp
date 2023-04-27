#ifndef RENDERING_RENDERGRAPH_HPP
#define RENDERING_RENDERGRAPH_HPP

#include <array>
#include <functional>
#include <map>
#include <optional>
#include <string>
#include <vector>

#include <vulkan/vulkan.hpp>

using RenderTargetRef = std::string;
using RenderAttachmentRef = std::string;
using RenderPassRef = std::string;
using RenderSubgraphRef = std::string;

using RenderPassAction = std::function<void(const vk::CommandBuffer &commandBuffer)>;
using RenderSubgraphCreateHandler = std::function<void(const vk::RenderPass &renderPass)>;
using RenderSubgraphDestroyHandler = std::function<void()>;

enum class RenderTargetType {
    Input = 1 << 0,
    Color = 1 << 1,
    DepthStencil = 1 << 2
};

enum class RenderTargetSource {
    Image,
    Swapchain
};

enum class RenderTargetFormat {
    DefaultColor,
    DefaultDepth,
    SwapchainColor
};

struct RenderTargetClearValue {
    std::array<float, 4> rgba;
    float depth;
    uint32_t stencil;
};

struct RenderTarget {
    RenderTargetType type;
    RenderTargetSource source;
    RenderTargetFormat format;
    RenderTargetClearValue clearValue;
};

struct RenderAttachment {
    uint32_t idx;
    RenderTargetRef targetRef;

    vk::AttachmentLoadOp loadOp;
    vk::AttachmentStoreOp storeOp;
    vk::ImageLayout initialLayout;
    vk::ImageLayout finalLayout;
};

struct RenderPass {
    uint32_t idx;

    std::vector<RenderAttachmentRef> inputRefs;
    std::vector<RenderAttachmentRef> colorRefs;
    std::optional<RenderAttachmentRef> depthRef;

    std::vector<RenderPassRef> dependencies;

    RenderPassAction action;
};

struct RenderSubgraph {
    std::map<RenderAttachmentRef, RenderAttachment> attachments;
    std::map<RenderPassRef, RenderPass> passes;
    RenderPassRef firstPass;

    std::vector<RenderSubgraphRef> next;

    RenderSubgraphCreateHandler createHandler;
    RenderSubgraphDestroyHandler destroyHandler;
};

struct RenderGraph {
    std::map<RenderTargetRef, RenderTarget> targets;
    std::map<RenderSubgraphRef, RenderSubgraph> subgraphs;
    RenderSubgraphRef firstSubgraph;
};

bool operator==(const RenderPass &lhs, const RenderPass &rhs);
bool operator==(const RenderAttachment &lhs, const RenderAttachment &rhs);
bool operator==(const RenderSubgraph &lhs, const RenderSubgraph &rhs);
bool operator==(const RenderTargetClearValue &lhs, const RenderTargetClearValue &rhs);
bool operator==(const RenderTarget &lhs, const RenderTarget &rhs);
bool operator==(const RenderGraph &lhs, const RenderGraph &rhs);

#endif // RENDERING_RENDERGRAPH_HPP
