#include "SubpassBuilder.hpp"

VkAttachmentReference *SubpassBuilder::toPtrArray(const std::vector<VkAttachmentReference> &vector) {
    VkAttachmentReference *refs = nullptr;

    if (vector.size() > 0) {
        refs = new VkAttachmentReference[vector.size()];
        memcpy(refs, vector.data(), vector.size() * sizeof(VkAttachmentReference));
    }

    return refs;
}

SubpassBuilder &SubpassBuilder::withInputAttachment(uint32_t idx, VkImageLayout layout) {
    this->_inputAttachments.push_back({idx, layout});

    return *this;
}

SubpassBuilder &SubpassBuilder::withColorAttachment(uint32_t idx, VkImageLayout layout) {
    this->_colorAttachments.push_back({idx, layout});

    return *this;
}

SubpassBuilder &SubpassBuilder::withResolveAttachment(uint32_t idx, VkImageLayout layout) {
    this->_resolveAttachments.push_back({idx, layout});

    return *this;
}

SubpassBuilder &SubpassBuilder::withDepthAttachment(uint32_t idx, VkImageLayout layout) {
    if (this->_depthAttachment != nullptr) {
        delete this->_depthAttachment;
    }

    this->_depthAttachment = new VkAttachmentReference{idx, layout};

    return *this;
}

VkSubpassDescription SubpassBuilder::build() {
    if (this->_resolveAttachments.size() != 0 &&
        this->_resolveAttachments.size() != this->_colorAttachments.size()) {
        throw std::runtime_error("Subpass should contain same amount of resolve attachments as color attachments");
    }

    return {
            .flags = 0,
            .pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS,
            .inputAttachmentCount = static_cast<uint32_t>(this->_inputAttachments.size()),
            .pInputAttachments = toPtrArray(this->_inputAttachments),
            .colorAttachmentCount = static_cast<uint32_t>(this->_colorAttachments.size()),
            .pColorAttachments = toPtrArray(this->_colorAttachments),
            .pResolveAttachments = toPtrArray(this->_resolveAttachments),
            .pDepthStencilAttachment = this->_depthAttachment,
            .preserveAttachmentCount = 0,
            .pPreserveAttachments = nullptr
    };
}
