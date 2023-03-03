#include "RenderpassBuilder.hpp"

#include "src/Rendering/Common.hpp"
#include "src/Rendering/PhysicalDevice.hpp"
#include "src/Rendering/RenderingDevice.hpp"
#include "src/Rendering/Builders/AttachmentBuilder.hpp"
#include "src/Rendering/Builders/SubpassBuilder.hpp"

RenderpassBuilder::RenderpassBuilder(RenderingDevice *renderingDevice)
        : _renderingDevice(renderingDevice) {
    //
}

RenderpassBuilder::~RenderpassBuilder() {
    for (VkSubpassDescription subpass: this->_subpasses) {
        if (subpass.pInputAttachments != nullptr) {
            delete[] subpass.pInputAttachments;
        }

        if (subpass.pColorAttachments != nullptr) {
            delete[] subpass.pColorAttachments;
        }

        if (subpass.pResolveAttachments != nullptr) {
            delete[] subpass.pResolveAttachments;
        }

        if (subpass.pDepthStencilAttachment != nullptr) {
            delete subpass.pDepthStencilAttachment;
        }
    }
}

RenderpassBuilder &RenderpassBuilder::addAttachment(AddAttachmentFunc func) {
    AttachmentBuilder builder(this->_renderingDevice->getPhysicalDevice());

    func(builder);

    this->_attachments.push_back(builder.build());

    return *this;
}

RenderpassBuilder &RenderpassBuilder::addSubpass(AddSubpassFunc func) {
    SubpassBuilder builder;

    func(builder);

    this->_subpasses.push_back(builder.build());

    return *this;
}

RenderpassBuilder &RenderpassBuilder::addSubpassDependency(uint32_t srcSubpass, uint32_t dstSubpass,
                                                           VkPipelineStageFlags srcStageMask,
                                                           VkPipelineStageFlags dstStageMask,
                                                           VkAccessFlags srcAccessMask,
                                                           VkAccessFlags dstAccessMask) {
    this->_dependencies.push_back(VkSubpassDependency{
            .srcSubpass = srcSubpass,
            .dstSubpass = dstSubpass,
            .srcStageMask = srcStageMask,
            .dstStageMask = dstStageMask,
            .srcAccessMask = srcAccessMask,
            .dstAccessMask = dstAccessMask,
            .dependencyFlags = VK_DEPENDENCY_BY_REGION_BIT
    });

    return *this;
}

VkRenderPass RenderpassBuilder::build() {
    return this->_renderingDevice->createRenderpass(this->_attachments, this->_subpasses, this->_dependencies);
}
