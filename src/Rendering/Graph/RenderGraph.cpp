#include "RenderGraph.hpp"

#include <optional>
#include <set>
#include <string_view>

#include <fmt/core.h>

#include "src/Engine/EngineError.hpp"
#include "src/Engine/Log.hpp"
#include "src/Rendering/GpuAllocator.hpp"
#include "src/Rendering/Swapchain.hpp"
#include "src/Rendering/Proxies/LogicalDeviceProxy.hpp"

static constexpr const std::string_view RENDER_GRAPH_TAG = "RenderGraph";

std::vector<vk::Framebuffer> RenderGraph::createFramebuffersFor(const RenderSubgraphRef &subgraphRef,
                                                                const RenderGraph::SubgraphInfo &subgraph) {
    std::vector<vk::Framebuffer> framebuffers;

    for (uint32_t imageIdx = 0; imageIdx < this->_swapchain->getImageCount(); imageIdx++) {
        auto attachments = std::vector<vk::ImageView>(subgraph.attachmentsMap.size());

        for (const auto &[targetRef, idx]: subgraph.attachmentsMap) {
            auto target = subgraph.subgraph.targets.at(targetRef);

            switch (target.source) {
                case IMAGE_RENDER_TARGET_SOURCE: {
                    auto it = this->_allocatedImages.find(targetRef);

                    if (it != this->_allocatedImages.end()) {
                        attachments[idx] = it->second->imageView;
                    } else {
                        auto requirements = subgraph.imageRequirements.at(targetRef);
                        auto imageView = this->_gpuAllocator->allocateImage(requirements).lock();

                        this->_allocatedImages[targetRef] = imageView;
                        attachments[idx] = imageView->imageView;
                    }
                    break;
                }

                case SWAPCHAIN_RENDER_TARGET_SOURCE:
                    attachments[idx] = this->_swapchain->getImageViews().at(imageIdx);
                    break;

                default:
                    throw EngineError("Incompatible render target source");
            }
        }

        auto createInfo = vk::FramebufferCreateInfo()
                .setWidth(this->_swapchain->getExtent().width)
                .setHeight(this->_swapchain->getExtent().height)
                .setLayers(1)
                .setRenderPass(this->_renderpasses[subgraphRef])
                .setAttachments(attachments);

        framebuffers.push_back(this->_logicalDevice->getHandle().createFramebuffer(createInfo));
    }

    return framebuffers;
}

RenderGraph::RenderGraph(const std::shared_ptr<Log> &log,
                         const std::shared_ptr<LogicalDeviceProxy> &logicalDevice,
                         const std::shared_ptr<GpuAllocator> &gpuAllocator,
                         const std::shared_ptr<Swapchain> &swapchain,
                         const std::map<RenderSubgraphRef, SubgraphInfo> &subgraphs,
                         const std::vector<RenderSubgraphRef> &executionOrder)
        : _log(log),
          _logicalDevice(logicalDevice),
          _gpuAllocator(gpuAllocator),
          _swapchain(swapchain),
          _subgraphs(subgraphs),
          _executionOrder(executionOrder) {
    //
}

RenderGraph::~RenderGraph() {
    for (const auto &[ref, subgraph]: this->_subgraphs) {
        for (const auto &depthReference: subgraph.depthReferences) {
            delete depthReference;
        }
    }
}

void RenderGraph::createRenderpasses() {
    for (const auto &[ref, subgraph]: this->_subgraphs) {
        auto createInfo = subgraph.renderPassCreateInfo;
        vk::RenderPass renderpass;

        try {
            renderpass = this->_logicalDevice->getHandle().createRenderPass(createInfo);
        } catch (const std::exception &error) {
            this->_log->error(RENDER_GRAPH_TAG, error);
            throw EngineError(fmt::format("Failed to create renderpass for subgraph {0}", ref));
        }

        this->_renderpasses[ref] = renderpass;

        subgraph.subgraph.createHandler(renderpass);
    }

    this->_graphBuilt = true;
}

void RenderGraph::destroyRenderpasses() {
    this->destroyFrameBuffers();

    for (const auto &[ref, renderpass]: this->_renderpasses) {
        this->_subgraphs[ref].subgraph.destroyHandler();

        this->_logicalDevice->getHandle().destroy(renderpass);
    }

    this->_renderpasses.clear();

    this->_graphBuilt = false;
}

void RenderGraph::createFramebuffers() {
    for (const auto &[ref, subgraph]: this->_subgraphs) {
        try {
            this->_framebuffers[ref] = this->createFramebuffersFor(ref, subgraph);
        } catch (const std::exception &error) {
            this->_log->error(RENDER_GRAPH_TAG, error);
            throw EngineError(fmt::format("Failed to create framebuffers for subgraph {0}", ref));
        }
    }

    this->_framebuffersBuilt = true;
}

void RenderGraph::destroyFrameBuffers() {
    for (const auto &[ref, framebuffers]: this->_framebuffers) {
        for (const auto &framebuffer: framebuffers) {
            this->_logicalDevice->getHandle().destroy(framebuffer);
        }
    }

    this->_framebuffers.clear();

    for (const auto &[targetRef, imageView]: this->_allocatedImages) {
        this->_gpuAllocator->freeImage(imageView);
    }

    this->_allocatedImages.clear();

    this->_framebuffersBuilt = false;
}

void RenderGraph::execute(uint32_t imageIdx, const vk::CommandBuffer &commandBuffer) {
    if (!this->_graphBuilt) {
        if (!this->_renderpasses.empty()) {
            this->destroyRenderpasses();
        }

        this->createRenderpasses();
    }

    if (!this->_framebuffersBuilt) {
        if (!this->_framebuffers.empty()) {
            this->destroyFrameBuffers();
        }

        this->createFramebuffers();
    }

    for (const auto &subgraphRef: this->_executionOrder) {
        auto subgraph = this->_subgraphs[subgraphRef];
        auto renderpass = this->_renderpasses[subgraphRef];
        auto framebuffer = this->_framebuffers[subgraphRef][imageIdx];

        auto beginInfo = vk::RenderPassBeginInfo()
                .setRenderPass(renderpass)
                .setFramebuffer(framebuffer)
                .setRenderArea(vk::Rect2D({}, this->_swapchain->getExtent()))
                .setClearValues(subgraph.clearValues);

        commandBuffer.beginRenderPass(beginInfo, vk::SubpassContents::eInline);

        for (auto it = subgraph.executionOrder.begin(); it != subgraph.executionOrder.end(); it++) {
            if (it != subgraph.executionOrder.begin()) {
                commandBuffer.nextSubpass(vk::SubpassContents::eInline);
            }

            auto passRef = *it;

            subgraph.subgraph.passes[passRef].action(commandBuffer);
        }

        commandBuffer.endRenderPass();
    }
}

const vk::RenderPass &RenderGraph::getRenderPass(const RenderSubgraphRef &subgraphRef) {
    auto it = this->_renderpasses.find(subgraphRef);

    if (it == this->_renderpasses.end()) {
        throw EngineError(fmt::format("Subgraph {0} not found in render graph", subgraphRef));
    }

    return it->second;
}

void RenderGraph::invalidateFramebuffers() {
    this->_framebuffersBuilt = false;
}
