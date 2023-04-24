#include "RenderGraph.hpp"

#include <optional>
#include <set>

#include <fmt/core.h>

#include "src/Engine/EngineError.hpp"
#include "src/Engine/Log.hpp"
#include "src/Rendering/GpuAllocator.hpp"
#include "src/Rendering/Swapchain.hpp"
#include "src/Rendering/Proxies/LogicalDeviceProxy.hpp"

vk::Format RenderGraph::toVulkanFormat(RenderTargetFormat format) {
    switch (format) {
        case DEFAULT_COLOR_RENDER_TARGET_FORMAT:
            return this->_swapchain->getColorFormat();

        case DEFAULT_DEPTH_RENDER_TARGET_FORMAT:
            return vk::Format::eD32Sfloat;

        default:
            throw EngineError("Not supported image format");
    }
}

void RenderGraph::createFramebufferFor(const RenderSubgraphRef &subgraphRef,
                                       const RenderGraph::ProcessedSubgraph &processedSubgraph) {
    std::vector<vk::Framebuffer> framebuffers;

    for (uint32_t imageIdx = 0; imageIdx < this->_swapchain->getImageCount(); imageIdx++) {
        auto attachments = std::vector<vk::ImageView>(processedSubgraph.attachmentsMap.size());

        for (const auto &[targetRef, idx]: processedSubgraph.attachmentsMap) {
            auto target = processedSubgraph.subgraph.targets.at(targetRef);

            switch (target.source) {
                case IMAGE_RENDER_TARGET_SOURCE: {
                    auto it = this->_allocatedImages.find(targetRef);

                    if (it != this->_allocatedImages.end()) {
                        attachments[idx] = it->second->imageView;
                    } else {
                        auto imageView = this->processTarget(target);

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
                .setRenderPass(processedSubgraph.renderPass)
                .setAttachments(attachments);

        framebuffers.push_back(this->_logicalDevice->getHandle().createFramebuffer(createInfo));
    }

    this->_framebuffers[subgraphRef] = framebuffers;
}

RenderGraph::ProcessedSubgraph RenderGraph::processSubgraph(const RenderSubgraph &subgraph) {
    std::map<RenderTargetRef, RenderPassRef> targetInputs;

    std::map<RenderTargetRef, vk::AttachmentDescription> unorderedAttachments;
    std::map<RenderTargetRef, uint32_t> attachmentsMap;
    uint32_t attachmentIdx = 0;

    std::vector<vk::AttachmentReference *> depthReferences;

    std::map<RenderPassRef, vk::SubpassDescription> unorderedSubpasses;
    std::map<RenderPassRef, uint32_t> subpassesMap;
    uint32_t subpassIdx = 0;

    std::vector<vk::SubpassDependency> dependencies;

    // 1. determine input/output targets, create attachments
    for (const auto &[passRef, pass]: subgraph.passes) {

        // 1.1. determine output targets
        for (const auto &output: pass.outputs) {
            auto it = targetInputs.find(output.targetRef);

            // 1.1.1 target should have only one input pass
            if (it != targetInputs.end()) {
                throw EngineError(fmt::format(
                        "Pass {0} trying to become output for target {1}, but pass {2} is already an output",
                        passRef, output.targetRef, it->second));
            }

            targetInputs[output.targetRef] = passRef;

            auto target = subgraph.tryGetTarget(output.targetRef);

            if (!target.has_value()) {
                throw EngineError(fmt::format("Target {0} not found in subgraph", output.targetRef));
            }

            // 1.1.2 create attachment
            unorderedAttachments[output.targetRef] = vk::AttachmentDescription()
                    .setFormat(this->toVulkanFormat(target->format))
                    .setLoadOp(output.loadOp)
                    .setStoreOp(output.storeOp)
                    .setInitialLayout(output.initialLayout)
                    .setFinalLayout(output.finalLayout);

            attachmentsMap[output.targetRef] = attachmentIdx++;
        }
    }

    // 2. create subpasses
    for (const auto &[passRef, pass]: subgraph.passes) {

        std::vector<vk::AttachmentReference> inputReferences;
        std::vector<vk::AttachmentReference> colorReferences;
        std::optional<vk::AttachmentReference *> depthReference;

        // 2.1. create input attachment refs, determine pass dependencies
        for (const auto &input: pass.inputs) {
            auto it = attachmentsMap.find(input.targetRef);

            if (it == attachmentsMap.end()) {
                throw EngineError(fmt::format("There is no attachment for input target {0} of pass {1}",
                                              input.targetRef, passRef));
            }

            inputReferences.emplace_back(it->second, vk::ImageLayout::eShaderReadOnlyOptimal);
        }

        // 2.2. create color and depth attachment refs
        for (const auto &output: pass.outputs) {
            auto target = subgraph.tryGetTarget(output.targetRef);

            if (!target.has_value()) {
                throw EngineError(fmt::format("Target {0} not found in subgraph", output.targetRef));
            }

            auto it = attachmentsMap.find(output.targetRef);

            if (it == attachmentsMap.end()) {
                throw EngineError(fmt::format("There is no attachment for output target {0} of pass {1}",
                                              output.targetRef, passRef));
            }

            switch (target->type) {
                case COLOR_RENDER_TARGET_TYPE:
                    colorReferences.emplace_back(it->second, vk::ImageLayout::eColorAttachmentOptimal);
                    break;

                case DEPTH_STENCIL_RENDER_TARGET_TYPE:
                    depthReference = new vk::AttachmentReference(it->second,
                                                                 vk::ImageLayout::eDepthStencilAttachmentOptimal);
                    break;

                default:
                    throw EngineError("Not supported render target type");
            }
        }

        if (depthReference.has_value()) {
            depthReferences.push_back(depthReference.value());
        }

        // 2.3. create subpass
        unorderedSubpasses[passRef] = vk::SubpassDescription()
                .setPipelineBindPoint(vk::PipelineBindPoint::eGraphics)
                .setInputAttachments(inputReferences)
                .setColorAttachments(colorReferences)
                .setPDepthStencilAttachment(depthReference.value_or(nullptr));

        subpassesMap[passRef] = subpassIdx++;
    }

    // 3. process dependencies
    for (const auto &[passRef, pass]: subgraph.passes) {

        uint32_t targetIdx = subpassesMap[passRef];

        // 3.1. if pass has no dependencies
        if (pass.inputs.empty()) {
            dependencies.emplace_back(VK_SUBPASS_EXTERNAL, targetIdx,
                                      vk::PipelineStageFlags(),
                                      vk::PipelineStageFlagBits::eColorAttachmentOutput,
                                      vk::AccessFlags(),
                                      vk::AccessFlagBits::eColorAttachmentRead |
                                      vk::AccessFlagBits::eColorAttachmentWrite);

            continue;
        }

        // 3.2. if pass has input targets
        for (const auto &input: pass.inputs) {
            auto it = targetInputs.find(input.targetRef);

            if (it == targetInputs.end()) {
                throw EngineError(fmt::format("There is no input pass for target {0}", input.targetRef));
            }

            uint32_t sourceIdx = subpassesMap[it->second];

            dependencies.emplace_back(sourceIdx, targetIdx,
                                      vk::PipelineStageFlagBits::eColorAttachmentOutput,
                                      vk::PipelineStageFlagBits::eColorAttachmentOutput,
                                      vk::AccessFlagBits::eColorAttachmentRead |
                                      vk::AccessFlagBits::eColorAttachmentWrite,
                                      vk::AccessFlagBits::eColorAttachmentRead |
                                      vk::AccessFlagBits::eColorAttachmentWrite);
        }
    }

    // 4. map collections and create renderpass
    auto attachments = std::vector<vk::AttachmentDescription>(unorderedAttachments.size());
    for (const auto &[targetRef, idx]: attachmentsMap) {
        attachments[idx] = unorderedAttachments[targetRef];
    }

    auto subpasses = std::vector<vk::SubpassDescription>(unorderedSubpasses.size());
    for (const auto &[passRef, idx]: subpassesMap) {
        subpasses[idx] = unorderedSubpasses[passRef];
    }

    auto createInfo = vk::RenderPassCreateInfo()
            .setAttachments(attachments)
            .setSubpasses(subpasses)
            .setDependencies(dependencies);

    return RenderGraph::ProcessedSubgraph{
            .subgraph = subgraph,
            .renderPass = this->_logicalDevice->getHandle().createRenderPass(createInfo),
            .attachmentsMap = attachmentsMap
    };
}

std::shared_ptr<ImageView> RenderGraph::processTarget(const RenderTarget &target) {
    if (target.source != IMAGE_RENDER_TARGET_SOURCE) {
        throw EngineError("Unable allocate image for render target with non-image source");
    }

    ImageRequirements requirements = {
            .usage = vk::ImageUsageFlags(),
            .memoryProperties = vk::MemoryPropertyFlagBits::eDeviceLocal,
            .extent = vk::Extent3D(this->_swapchain->getExtent(), 1),
            .format = this->toVulkanFormat(target.format),
            .layerCount = 1,
            .samples = vk::SampleCountFlagBits::e1,
            .imageFlags = std::nullopt,
            .type = vk::ImageViewType::e2D,
            .aspectMask = std::nullopt
    };

    if (target.type == INPUT_RENDER_TARGET_TYPE) {
        requirements.usage |= vk::ImageUsageFlagBits::eInputAttachment;
    }

    if (target.type == COLOR_RENDER_TARGET_TYPE) {
        requirements.usage |= vk::ImageUsageFlagBits::eColorAttachment;
        requirements.aspectMask = requirements.aspectMask.value_or(vk::ImageAspectFlags()) |
                                  vk::ImageAspectFlagBits::eColor;
    }

    if (target.type == DEPTH_STENCIL_RENDER_TARGET_TYPE) {
        requirements.usage |= vk::ImageUsageFlagBits::eDepthStencilAttachment;
        requirements.aspectMask = requirements.aspectMask.value_or(vk::ImageAspectFlags()) |
                                  vk::ImageAspectFlagBits::eDepth;
    }

    return this->_gpuAllocator->allocateImage(requirements).lock();
}

RenderGraph::RenderGraph(const std::shared_ptr<Log> &log,
                         const std::shared_ptr<LogicalDeviceProxy> &logicalDevice,
                         const std::shared_ptr<GpuAllocator> &gpuAllocator,
                         const std::shared_ptr<Swapchain> &swapchain)
        : _log(log),
          _logicalDevice(logicalDevice),
          _gpuAllocator(gpuAllocator),
          _swapchain(swapchain) {
    //
}

void RenderGraph::createGraph() {
    for (const auto &[ref, subgraph]: this->_subgraphs) {
        this->_processedSubgraphs[ref] = this->processSubgraph(subgraph);
    }
}

void RenderGraph::destroyGraph() {
    this->destroyFrameBuffers();

    for (const auto &[subgraphRef, processedSubgraph]: this->_processedSubgraphs) {
        this->_logicalDevice->getHandle().destroy(processedSubgraph.renderPass);
    }

    this->_processedSubgraphs.clear();
}

void RenderGraph::createFramebuffers() {
    for (const auto &[subgraphRef, processedSubgraph]: this->_processedSubgraphs) {
        this->createFramebufferFor(subgraphRef, processedSubgraph);
    }
}

void RenderGraph::destroyFrameBuffers() {
    for (const auto &[subgraphRef, framebuffers]: this->_framebuffers) {
        for (const auto &framebuffer: framebuffers) {
            this->_logicalDevice->getHandle().destroy(framebuffer);
        }
    }

    this->_framebuffers.clear();

    for (const auto &[targetRef, imageView]: this->_allocatedImages) {
        this->_gpuAllocator->freeImage(imageView);
    }

    this->_allocatedImages.clear();
}

void RenderGraph::execute(uint32_t imageIdx, const vk::CommandBuffer &commandBuffer) {
    for (const auto &subgraphRef: this->_subgraphExecutionOrder) {
        // TODO
    }
}

const vk::RenderPass &RenderGraph::getRenderPass(const RenderSubgraphRef &subgraphRef) {
    auto it = this->_processedSubgraphs.find(subgraphRef);

    if (it == this->_processedSubgraphs.end()) {
        throw EngineError(fmt::format("Subgraph {} not found in render graph", subgraphRef));
    }

    return it->second.renderPass;
}
