#include "RenderGraphExecutor.hpp"

#include <queue>
#include <string_view>

#include <fmt/core.h>

#include "src/Engine/EngineError.hpp"
#include "src/Rendering/GpuAllocator.hpp"
#include "src/Rendering/Swapchain.hpp"
#include "src/Rendering/Proxies/LogicalDeviceProxy.hpp"

vk::Format RenderGraphExecutor::processFormat(const RenderTargetFormat &format) {
    switch (format) {
        case RenderTargetFormat::DefaultColor:
            return vk::Format::eR8G8B8A8Srgb;

        case RenderTargetFormat::DefaultDepth:
            return vk::Format::eD32Sfloat;

        case RenderTargetFormat::SwapchainColor:
            return this->_swapchain->getColorFormat();

        default:
            throw EngineError("Unsupported image format");
    }
}

vk::RenderPass RenderGraphExecutor::processSubgraphRenderpass(const RenderSubgraph &subgraph) {
    std::map<RenderPassRef, std::vector<vk::AttachmentReference>> inputAttachmentsMap;
    std::map<RenderPassRef, std::vector<vk::AttachmentReference>> colorAttachmentsMap;
    std::map<RenderPassRef, vk::AttachmentReference *> depthAttachmentsMap;

    auto attachments = std::vector<vk::AttachmentDescription>(subgraph.attachments.size());
    auto subpasses = std::vector<vk::SubpassDescription>(subgraph.passes.size());
    std::vector<vk::SubpassDependency> dependencies;

    for (const auto &[attachmentRef, attachment]: subgraph.attachments) {
        auto targetIterator = this->_graph.targets.find(attachment.targetRef);

        if (targetIterator == this->_graph.targets.end()) {
            throw EngineError(fmt::format("Attachment {0}: unknown target {1}", attachmentRef, attachment.targetRef));
        }

        attachments[attachment.idx] = vk::AttachmentDescription()
                .setFormat(this->processFormat(targetIterator->second.format))
                .setSamples(vk::SampleCountFlagBits::e1)
                .setLoadOp(attachment.loadOp)
                .setStoreOp(attachment.storeOp)
                .setInitialLayout(attachment.initialLayout)
                .setFinalLayout(attachment.finalLayout);
    }

    for (const auto &[passRef, pass]: subgraph.passes) {
        inputAttachmentsMap[passRef] = std::vector<vk::AttachmentReference>();
        colorAttachmentsMap[passRef] = std::vector<vk::AttachmentReference>();
        depthAttachmentsMap[passRef] = nullptr;

        for (const auto &inputRef: pass.inputRefs) {
            auto attachmentIterator = subgraph.attachments.find(inputRef);

            if (attachmentIterator == subgraph.attachments.end()) {
                throw EngineError(fmt::format("Pass {0}: unknown input attachment {1}", passRef, inputRef));
            }

            inputAttachmentsMap[passRef].push_back(vk::AttachmentReference(attachmentIterator->second.idx,
                                                                           vk::ImageLayout::eShaderReadOnlyOptimal));
        }

        for (const auto &colorRef: pass.colorRefs) {
            auto attachmentIterator = subgraph.attachments.find(colorRef);

            if (attachmentIterator == subgraph.attachments.end()) {
                throw EngineError(fmt::format("Pass {0}: unknown color attachment {1}", passRef, colorRef));
            }

            colorAttachmentsMap[passRef].push_back(vk::AttachmentReference(attachmentIterator->second.idx,
                                                                           vk::ImageLayout::eColorAttachmentOptimal));
        }

        if (pass.depthRef.has_value()) {
            auto attachmentIterator = subgraph.attachments.find(pass.depthRef.value());

            if (attachmentIterator == subgraph.attachments.end()) {
                throw EngineError(fmt::format("Pass {0}: unknown depth attachment {1}", passRef,
                                              pass.depthRef.value()));
            }

            depthAttachmentsMap[passRef] = new vk::AttachmentReference(attachmentIterator->second.idx,
                                                                       vk::ImageLayout::eDepthStencilAttachmentOptimal);
        }

        subpasses[pass.idx] = vk::SubpassDescription()
                .setPipelineBindPoint(vk::PipelineBindPoint::eGraphics)
                .setInputAttachments(inputAttachmentsMap[passRef])
                .setColorAttachments(colorAttachmentsMap[passRef])
                .setPDepthStencilAttachment(depthAttachmentsMap[passRef]);

        if (pass.dependencies.empty()) {
            auto dependency = vk::SubpassDependency()
                    .setSrcSubpass(VK_SUBPASS_EXTERNAL)
                    .setDstSubpass(pass.idx)
                    .setSrcStageMask(vk::PipelineStageFlags())
                    .setDstStageMask(vk::PipelineStageFlagBits::eColorAttachmentOutput)
                    .setSrcAccessMask(vk::AccessFlags())
                    .setDstAccessMask(vk::AccessFlagBits::eColorAttachmentWrite |
                                      vk::AccessFlagBits::eColorAttachmentRead);

            dependencies.push_back(dependency);
        } else {
            for (const auto &dependencyPassRef: pass.dependencies) {
                auto dependencyPassIterator = subgraph.passes.find(dependencyPassRef);

                if (dependencyPassIterator == subgraph.passes.end()) {
                    throw EngineError(fmt::format("Pass {0}: unknown dependency pass {1}", passRef,
                                                  dependencyPassRef));
                }

                auto dependency = vk::SubpassDependency()
                        .setSrcSubpass(dependencyPassIterator->second.idx)
                        .setDstSubpass(pass.idx)
                        .setSrcStageMask(vk::PipelineStageFlagBits::eColorAttachmentOutput)
                        .setDstStageMask(vk::PipelineStageFlagBits::eColorAttachmentOutput)
                        .setSrcAccessMask(vk::AccessFlagBits::eColorAttachmentWrite |
                                          vk::AccessFlagBits::eColorAttachmentRead)
                        .setDstAccessMask(vk::AccessFlagBits::eColorAttachmentWrite |
                                          vk::AccessFlagBits::eColorAttachmentRead);

                dependencies.push_back(dependency);
            }
        }
    }

    auto createInfo = vk::RenderPassCreateInfo()
            .setAttachments(attachments)
            .setSubpasses(subpasses)
            .setDependencies(dependencies);

    auto renderpass = this->_logicalDevice->getHandle().createRenderPass(createInfo);

    for (const auto &[passRef, depthAttachment]: depthAttachmentsMap) {
        delete depthAttachment;
    }

    return renderpass;
}

RenderGraphExecutor::ExecutionOrder RenderGraphExecutor::processSubgraphExecutionOrder(const RenderSubgraph &subgraph) {
    auto executionOrder = std::vector<RenderPassRef>(subgraph.passes.size());

    for (const auto &[passRef, pass]: subgraph.passes) {
        executionOrder[pass.idx] = passRef;
    }

    return executionOrder;
}

RenderGraphExecutor::FramebufferCollection RenderGraphExecutor::processSubgraphFramebuffers(
        const vk::RenderPass &renderpass,
        const RenderSubgraph &subgraph) {
    auto getAllocatedImageViewFor = [&](
            const RenderTargetRef &targetRef,
            const RenderTarget &target) -> vk::ImageView {
        auto it = this->_images.find(targetRef);

        if (it != this->_images.end()) {
            return it->second->imageView;
        }

        ImageRequirements requirements = {
                .usage = vk::ImageUsageFlags(),
                .memoryProperties = vk::MemoryPropertyFlagBits::eDeviceLocal,
                .extent = vk::Extent3D(this->_swapchain->getExtent(), 1),
                .format = this->processFormat(target.format),
                .layerCount = 1,
                .samples = vk::SampleCountFlagBits::e1,
                .imageFlags = std::nullopt,
                .type = vk::ImageViewType::e2D,
                .aspectMask = std::nullopt
        };

        if (target.type == RenderTargetType::Input) {
            requirements.usage |= vk::ImageUsageFlagBits::eInputAttachment;
        }

        if (target.type == RenderTargetType::Color) {
            requirements.usage |= vk::ImageUsageFlagBits::eColorAttachment;
            requirements.aspectMask = requirements.aspectMask.value_or(vk::ImageAspectFlags()) |
                                      vk::ImageAspectFlagBits::eColor;
        }

        if (target.type == RenderTargetType::DepthStencil) {
            requirements.usage |= vk::ImageUsageFlagBits::eDepthStencilAttachment;
            requirements.aspectMask = requirements.aspectMask.value_or(vk::ImageAspectFlags()) |
                                      vk::ImageAspectFlagBits::eDepth;
        }

        auto image = this->_gpuAllocator->allocateImage(requirements).lock();
        this->_images[targetRef] = image;

        return image->imageView;
    };

    auto framebuffers = std::vector<vk::Framebuffer>(this->_swapchain->getImageCount());

    for (uint32_t imageIdx = 0; imageIdx < this->_swapchain->getImageCount(); ++imageIdx) {
        auto attachments = std::vector<vk::ImageView>(subgraph.attachments.size());

        for (const auto &[attachmentRef, attachment]: subgraph.attachments) {
            auto target = this->_graph.targets.at(attachment.targetRef);

            switch (target.source) {
                case RenderTargetSource::Image:
                    attachments[attachment.idx] = getAllocatedImageViewFor(attachment.targetRef, target);
                    break;

                case RenderTargetSource::Swapchain:
                    attachments[attachment.idx] = this->_swapchain->getImageViews().at(imageIdx);
                    break;
            }
        }

        auto createInfo = vk::FramebufferCreateInfo()
                .setRenderPass(renderpass)
                .setAttachments(attachments)
                .setWidth(this->_swapchain->getExtent().width)
                .setHeight(this->_swapchain->getExtent().height)
                .setLayers(1);

        framebuffers[imageIdx] = this->_logicalDevice->getHandle().createFramebuffer(createInfo);
    }

    return framebuffers;
}

void RenderGraphExecutor::createFramebuffers() {
    for (const auto &[subgraphRef, subgraph]: this->_graph.subgraphs) {
        try {
            this->_framebuffers[subgraphRef] = this->processSubgraphFramebuffers(this->_renderpasses[subgraphRef],
                                                                                 subgraph);
        } catch (const std::exception &error) {
            throw EngineError(fmt::format("Failed to create framebuffers for {0}: {1}", subgraphRef, error.what()));
        }
    }
}

void RenderGraphExecutor::destroyFramebuffers() {
    for (const auto &[subgraphRef, framebuffers]: this->_framebuffers) {
        for (const auto &framebuffer: framebuffers) {
            this->_logicalDevice->getHandle().destroy(framebuffer);
        }
    }

    this->_framebuffers.clear();
}

void RenderGraphExecutor::executeSubgraph(const RenderSubgraphRef &subgraphRef,
                                          const RenderSubgraph &subgraph,
                                          uint32_t imageIdx,
                                          const vk::CommandBuffer &commandBuffer) {
    auto clearValues = std::vector<vk::ClearValue>(subgraph.attachments.size());
    for (const auto &[attachmentRef, attachment]: subgraph.attachments) {
        auto clearValue = this->_graph.targets[attachment.targetRef].clearValue;

        clearValues[attachment.idx] = vk::ClearValue()
                .setColor(vk::ClearColorValue(clearValue.rgba))
                .setDepthStencil(vk::ClearDepthStencilValue(clearValue.depth, clearValue.stencil));
    }

    auto beginInfo = vk::RenderPassBeginInfo()
            .setRenderPass(this->_renderpasses[subgraphRef])
            .setFramebuffer(this->_framebuffers[subgraphRef][imageIdx])
            .setRenderArea(vk::Rect2D(0, this->_swapchain->getExtent()))
            .setClearValues(clearValues);

    commandBuffer.beginRenderPass(beginInfo, vk::SubpassContents::eInline);

    auto executionOrder = this->_executionOrders[subgraphRef];

    for (auto it = executionOrder.begin(); it != executionOrder.end(); it++) {
        if (it != executionOrder.begin()) {
            commandBuffer.nextSubpass(vk::SubpassContents::eInline);
        }

        auto passIterator = subgraph.passes.find(*it);

        if (passIterator == subgraph.passes.end()) {
            throw EngineError(fmt::format("Unknown pass {0}", subgraphRef));
        }

        passIterator->second.action(commandBuffer);
    }

    commandBuffer.endRenderPass();
}

RenderGraphExecutor::RenderGraphExecutor(const std::shared_ptr<GpuAllocator> &gpuAllocator,
                                         const std::shared_ptr<Swapchain> &swapchain,
                                         const std::shared_ptr<LogicalDeviceProxy> &logicalDevice,
                                         const RenderGraph &graph)
        : _gpuAllocator(gpuAllocator),
          _swapchain(swapchain),
          _logicalDevice(logicalDevice),
          _graph(graph) {
    //
}

void RenderGraphExecutor::create() {
    for (const auto &[subgraphRef, subgraph]: this->_graph.subgraphs) {
        vk::RenderPass renderpass;

        try {
            renderpass = this->processSubgraphRenderpass(subgraph);
            subgraph.createHandler(renderpass);
        } catch (const std::exception &error) {
            throw EngineError(fmt::format("Failed to create renderpass for {0}: {1}", subgraphRef, error.what()));
        }

        this->_renderpasses[subgraphRef] = renderpass;
        this->_executionOrders[subgraphRef] = this->processSubgraphExecutionOrder(subgraph);
    }

    this->createFramebuffers();
}

void RenderGraphExecutor::destroy() {
    this->destroyFramebuffers();

    for (const auto &[subgraphRef, renderpass]: this->_renderpasses) {
        this->_graph.subgraphs[subgraphRef].destroyHandler();
        this->_logicalDevice->getHandle().destroy(renderpass);
    }

    this->_renderpasses.clear();
}

void RenderGraphExecutor::recreateFrameBuffers() {
    this->destroyFramebuffers();
    this->createFramebuffers();
}

void RenderGraphExecutor::execute(uint32_t imageIdx, const vk::CommandBuffer &commandBuffer) {
    std::queue<RenderSubgraphRef> subgraphQueue;
    subgraphQueue.push(this->_graph.firstSubgraph);

    while (!subgraphQueue.empty()) {
        auto subgraphRef = subgraphQueue.front();
        subgraphQueue.pop();

        auto subgraph = this->_graph.subgraphs[subgraphRef];

        this->executeSubgraph(subgraphRef, subgraph, imageIdx, commandBuffer);

        for (const auto &nextSubgraphRef: subgraph.next) {
            subgraphQueue.push(nextSubgraphRef);
        }
    }
}
