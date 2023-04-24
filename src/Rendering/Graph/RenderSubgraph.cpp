#include "RenderSubgraph.hpp"

std::optional<RenderTarget> RenderSubgraph::tryGetTarget(const RenderTargetRef &targetRef) const {
    auto it = this->targets.find(targetRef);

    if (it == this->targets.end()) {
        return std::nullopt;
    }

    return it->second;
}
