#ifndef RENDERING_GRAPH_RENDERSUBGRAPH_HPP
#define RENDERING_GRAPH_RENDERSUBGRAPH_HPP

#include <functional>
#include <map>
#include <optional>
#include <string>

#include <vulkan/vulkan.hpp>

#include "src/Rendering/Graph/RenderPass.hpp"
#include "src/Rendering/Graph/RenderTarget.hpp"

using RenderSubgraphRef = std::string;

using RenderSubgraphCreateHandler = std::function<void(const vk::RenderPass &renderPass)>;
using RenderSubgraphDestroyHandler = std::function<void()>;

struct RenderSubgraph {
    std::vector<RenderSubgraphRef> dependencies;
    RenderSubgraphCreateHandler createHandler;
    RenderSubgraphDestroyHandler destroyHandler;

    std::map<RenderTargetRef, RenderTarget> targets;
    std::map<RenderPassRef, RenderPass> passes;

    [[nodiscard]] std::optional<RenderTarget> tryGetTarget(const RenderTargetRef &targetRef) const;
};

#endif // RENDERING_GRAPH_RENDERSUBGRAPH_HPP
