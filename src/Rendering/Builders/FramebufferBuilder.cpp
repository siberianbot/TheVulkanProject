#include "FramebufferBuilder.hpp"

#include "src/Rendering/RenderingDevice.hpp"

FramebufferBuilder::FramebufferBuilder(RenderingDevice *renderingDevice, VkRenderPass renderpass)
        : _renderingDevice(renderingDevice),
          _renderpass(renderpass) {
    //
}

FramebufferBuilder &FramebufferBuilder::addAttachment(VkImageView imageView) {
    this->_attachments.push_back(imageView);

    return *this;
}

FramebufferBuilder &FramebufferBuilder::replaceAttachment(uint32_t idx, VkImageView imageView) {
    if (idx >= this->_attachments.size()) {
        throw std::runtime_error("Attachment index is out of range");
    }

    this->_attachments[idx] = imageView;

    return *this;
}

FramebufferBuilder &FramebufferBuilder::withExtent(VkExtent2D extent) {
    this->_extent = extent;

    return *this;
}

VkFramebuffer FramebufferBuilder::build() {
    if (!this->_extent.has_value()) {
        throw std::runtime_error("Extent is required");
    }

    return this->_renderingDevice->createFramebuffer(this->_renderpass, this->_extent.value(), this->_attachments);
}
