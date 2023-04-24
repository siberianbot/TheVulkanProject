#ifndef RENDERING_GRAPH_RENDERSUBGRAPH_HPP
#define RENDERING_GRAPH_RENDERSUBGRAPH_HPP

#include <map>
#include <optional>
#include <string>

#include "src/Rendering/Graph/RenderPass.hpp"

using RenderSubgraphRef = std::string;

struct RenderSubgraph {
    std::map<RenderTargetRef, RenderTarget> targets;
    std::map<RenderPassRef, RenderPass> passes;

    [[nodiscard]] std::optional<RenderTarget> tryGetTarget(const RenderTargetRef &targetRef) const;
};

#endif // RENDERING_GRAPH_RENDERSUBGRAPH_HPP
