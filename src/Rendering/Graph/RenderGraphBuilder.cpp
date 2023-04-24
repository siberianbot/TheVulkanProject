#include "RenderGraphBuilder.hpp"

#include <fmt/core.h>

#include "src/Engine/EngineError.hpp"
#include "src/Rendering/Swapchain.hpp"
#include "src/Utils/TopologicalSort.hpp"

vk::Format RenderGraphBuilder::toVulkanFormat(RenderTargetFormat format) {
    switch (format) {
        case DEFAULT_COLOR_RENDER_TARGET_FORMAT:
            return this->_swapchain->getColorFormat();

        case DEFAULT_DEPTH_RENDER_TARGET_FORMAT:
            return vk::Format::eD32Sfloat;

        default:
            throw EngineError("Not supported image format");
    }
}

RenderGraph::SubgraphInfo RenderGraphBuilder::processSubgraph(const RenderSubgraph &subgraph) {

    std::map<RenderPassRef, std::vector<RenderTargetRef>> passOutputs;

    std::map<RenderTargetRef, vk::AttachmentDescription> unorderedAttachments;
    std::map<RenderTargetRef, uint32_t> attachmentsMap;
    uint32_t attachmentIdx = 0;

    // 1. create attachments and define pass outputs
    for (const auto &[targetRef, target]: subgraph.targets) {
        unorderedAttachments[targetRef] = vk::AttachmentDescription()
                .setFormat(this->toVulkanFormat(target.format))
                .setLoadOp(target.input.loadOp)
                .setStoreOp(target.input.storeOp)
                .setInitialLayout(target.input.initialLayout)
                .setFinalLayout(target.input.finalLayout);

        attachmentsMap[targetRef] = attachmentIdx++;

        passOutputs[target.input.passRef].push_back(targetRef);
    }

    std::vector<vk::AttachmentReference *> depthReferences;

    std::map<RenderPassRef, vk::SubpassDescription> unorderedSubpasses;
    std::map<RenderPassRef, uint32_t> subpassesMap;
    uint32_t subpassIdx = 0;

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
        for (const auto &outputRef: passOutputs[passRef]) {
            auto target = subgraph.tryGetTarget(outputRef);

            if (!target.has_value()) {
                throw EngineError(fmt::format("Target {0} not found in subgraph", outputRef));
            }

            auto it = attachmentsMap.find(outputRef);

            if (it == attachmentsMap.end()) {
                throw EngineError(fmt::format("There is no attachment for output target {0} of pass {1}",
                                              outputRef, passRef));
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
    }

    std::vector<RenderPassRef> executionOrder;

    // 3. define execution order
    {
        auto sort = TopologicalSort<RenderPassRef>([&]() {
            auto items = std::vector<RenderPassRef>(subgraph.passes.size());
            std::transform(subgraph.passes.begin(), subgraph.passes.end(), items.begin(),
                           [](const std::pair<RenderPassRef, RenderPass> &item) {
                               return item.first;
                           });

            return items;
        }, [&](const RenderPassRef &item) {
            std::vector<RenderPassRef> dependencies;

            for (const auto &input: subgraph.passes.at(item).inputs) {
                auto it = subgraph.targets.find(input.targetRef);

                if (it == subgraph.targets.end()) {
                    throw EngineError(fmt::format("There is no input pass for target {0}", input.targetRef));
                }

                dependencies.push_back(it->second.input.passRef);
            }

            return dependencies;
        });

        sort.sort();

        executionOrder = sort.getResult();
    }

    std::vector<vk::SubpassDependency> dependencies;

    // 4. process dependencies according to execution order
    for (const auto &passRef: executionOrder) {
        auto pass = subgraph.passes.at(passRef);
        uint32_t targetIdx = subpassIdx++;
        subpassesMap[passRef] = targetIdx;

        // 4.1. if pass has no dependencies
        if (pass.inputs.empty()) {
            dependencies.emplace_back(VK_SUBPASS_EXTERNAL, targetIdx,
                                      vk::PipelineStageFlags(),
                                      vk::PipelineStageFlagBits::eColorAttachmentOutput,
                                      vk::AccessFlags(),
                                      vk::AccessFlagBits::eColorAttachmentRead |
                                      vk::AccessFlagBits::eColorAttachmentWrite);

            continue;
        }

        // 4.2. if pass has input targets
        for (const auto &input: pass.inputs) {
            auto it = subgraph.targets.find(input.targetRef);

            if (it == subgraph.targets.end()) {
                throw EngineError(fmt::format("There is no input pass for target {0}", input.targetRef));
            }

            uint32_t sourceIdx = subpassesMap[it->second.input.passRef];

            dependencies.emplace_back(sourceIdx, targetIdx,
                                      vk::PipelineStageFlagBits::eColorAttachmentOutput,
                                      vk::PipelineStageFlagBits::eColorAttachmentOutput,
                                      vk::AccessFlagBits::eColorAttachmentRead |
                                      vk::AccessFlagBits::eColorAttachmentWrite,
                                      vk::AccessFlagBits::eColorAttachmentRead |
                                      vk::AccessFlagBits::eColorAttachmentWrite);
        }
    }

    // 5. map collections for renderpass creation
    auto attachments = std::vector<vk::AttachmentDescription>(unorderedAttachments.size());
    for (const auto &[targetRef, idx]: attachmentsMap) {
        attachments[idx] = unorderedAttachments[targetRef];
    }

    auto subpasses = std::vector<vk::SubpassDescription>(unorderedSubpasses.size());
    for (const auto &[passRef, idx]: subpassesMap) {
        subpasses[idx] = unorderedSubpasses[passRef];
    }

    auto renderPassCreateInfo = vk::RenderPassCreateInfo()
            .setAttachments(attachments)
            .setSubpasses(subpasses)
            .setDependencies(dependencies);

    // 6. collect image requirements
    std::map<RenderTargetRef, ImageRequirements> imageRequirements;
    for (const auto &[targetRef, target]: subgraph.targets) {
        imageRequirements[targetRef] = this->processTarget(target);
    }

    // 7. collect clear values
    auto clearValues = std::vector<vk::ClearValue>(subgraph.targets.size());
    for (const auto &[targetRef, target]: subgraph.targets) {
        uint32_t idx = attachmentsMap[targetRef];
        clearValues[idx] = target.clearValue;
    }

    return RenderGraph::SubgraphInfo{
            .subgraph = subgraph,
            .attachmentsMap = attachmentsMap,
            .imageRequirements = imageRequirements,
            .renderPassCreateInfo = renderPassCreateInfo,
            .executionOrder = executionOrder,
            .depthReferences = depthReferences,
            .clearValues = clearValues
    };
}

ImageRequirements RenderGraphBuilder::processTarget(const RenderTarget &target) {
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

    return requirements;
}

std::vector<RenderSubgraphRef> RenderGraphBuilder::processExecutionOrder() {
    auto sort = TopologicalSort<RenderSubgraphRef>([&]() {
        auto items = std::vector<RenderSubgraphRef>(this->_subgraphs.size());
        std::transform(this->_subgraphs.begin(), this->_subgraphs.end(), items.begin(),
                       [](const std::pair<RenderSubgraphRef, RenderSubgraph> &item) {
                           return item.first;
                       });

        return items;
    }, [&](const RenderSubgraphRef &item) {
        return this->_subgraphs[item].dependencies;
    });

    sort.sort();

    return sort.getResult();
}

RenderGraphBuilder::RenderGraphBuilder(const std::shared_ptr<Log> &log,
                                       const std::shared_ptr<LogicalDeviceProxy> &logicalDevice,
                                       const std::shared_ptr<GpuAllocator> &gpuAllocator,
                                       const std::shared_ptr<Swapchain> &swapchain)
        : _log(log),
          _logicalDevice(logicalDevice),
          _gpuAllocator(gpuAllocator),
          _swapchain(swapchain) {
    //
}

RenderGraphBuilder &RenderGraphBuilder::addSubgraph(const RenderSubgraphRef &ref, const RenderSubgraph &subgraph) {
    if (this->_subgraphs.contains(ref)) {
        throw EngineError(fmt::format("Subgraph {0} already defined", ref));
    }

    this->_subgraphs[ref] = subgraph;

    return *this;
}

std::shared_ptr<RenderGraph> RenderGraphBuilder::build() {
    std::map<RenderSubgraphRef, RenderGraph::SubgraphInfo> subgraphs;
    for (const auto &[ref, subgraph]: this->_subgraphs) {
        try {
            subgraphs[ref] = this->processSubgraph(subgraph);
        } catch (const std::exception &error) {
            throw EngineError(fmt::format("Failed to process subgraph {0}: {1}", ref, error.what()));
        }
    }

    std::vector<RenderSubgraphRef> executionOrder = this->processExecutionOrder();

    return std::make_shared<RenderGraph>(this->_log,
                                         this->_logicalDevice,
                                         this->_gpuAllocator,
                                         this->_swapchain,
                                         subgraphs,
                                         executionOrder);
}
