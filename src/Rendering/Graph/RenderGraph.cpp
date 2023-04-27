#include "RenderGraph.hpp"

// TODO: std::function is not comparable

bool operator==(const RenderPass &lhs, const RenderPass &rhs) {
    // see TODO
    return lhs.idx == rhs.idx &&
           lhs.dependencies == rhs.dependencies &&
           lhs.inputRefs == rhs.inputRefs &&
           lhs.colorRefs == rhs.colorRefs &&
           lhs.depthRef == rhs.depthRef;
}

bool operator==(const RenderAttachment &lhs, const RenderAttachment &rhs) {
    return lhs.idx == rhs.idx &&
           lhs.targetRef == rhs.targetRef &&
           lhs.loadOp == rhs.loadOp &&
           lhs.storeOp == rhs.storeOp &&
           lhs.initialLayout == rhs.initialLayout &&
           lhs.finalLayout == rhs.finalLayout;
}

bool operator==(const RenderSubgraph &lhs, const RenderSubgraph &rhs) {
    // see TODO
    return lhs.attachments == rhs.attachments &&
           lhs.passes == rhs.passes &&
           lhs.next == rhs.next &&
           lhs.firstPass == rhs.firstPass;
}

bool operator==(const RenderTargetClearValue &lhs, const RenderTargetClearValue &rhs) {
    return lhs.rgba == rhs.rgba &&
           lhs.depth == rhs.depth &&
           lhs.stencil == rhs.stencil;
}

bool operator==(const RenderTarget &lhs, const RenderTarget &rhs) {
    return lhs.type == rhs.type &&
           lhs.source == rhs.source &&
           lhs.format == rhs.format &&
           lhs.clearValue == rhs.clearValue;
}

bool operator==(const RenderGraph &lhs, const RenderGraph &rhs) {
    return lhs.targets == rhs.targets &&
           lhs.subgraphs == rhs.subgraphs &&
           lhs.firstSubgraph == rhs.firstSubgraph;
}
