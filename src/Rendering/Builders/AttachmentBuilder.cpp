#include "AttachmentBuilder.hpp"

#include "src/Rendering/PhysicalDevice.hpp"

AttachmentBuilder::AttachmentBuilder(PhysicalDevice *physicalDevice)
        : _physicalDevice(physicalDevice) {
    //
}

AttachmentBuilder &AttachmentBuilder::clear() {
    this->_loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;

    return *this;
}

AttachmentBuilder &AttachmentBuilder::load() {
    this->_loadOp = VK_ATTACHMENT_LOAD_OP_LOAD;

    return *this;
}

AttachmentBuilder &AttachmentBuilder::withFormat(VkFormat format) {
    this->_format = format;

    return *this;
}

AttachmentBuilder &AttachmentBuilder::withSamples(VkSampleCountFlagBits samples) {
    this->_samples = samples;

    return *this;
}

AttachmentBuilder &AttachmentBuilder::withInitialLayout(VkImageLayout layout) {
    this->_initialLayout = layout;

    return *this;
}

AttachmentBuilder &AttachmentBuilder::withFinalLayout(VkImageLayout layout) {
    this->_finalLayout = layout;

    return *this;
}

AttachmentBuilder &AttachmentBuilder::defaultColorAttachment(bool clear) {
    this->_initialLayout = clear
                           ? VK_IMAGE_LAYOUT_UNDEFINED
                           : VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
    this->_finalLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
    this->_loadOp = clear
                    ? VK_ATTACHMENT_LOAD_OP_CLEAR
                    : VK_ATTACHMENT_LOAD_OP_LOAD;
    this->_format = this->_physicalDevice->getColorFormat();
    this->_samples = this->_physicalDevice->getMsaaSamples();

    return *this;
}

AttachmentBuilder &AttachmentBuilder::defaultDepthAttachment(bool clear) {
    this->_initialLayout = clear
                           ? VK_IMAGE_LAYOUT_UNDEFINED
                           : VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;
    this->_finalLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;
    this->_loadOp = clear
                    ? VK_ATTACHMENT_LOAD_OP_CLEAR
                    : VK_ATTACHMENT_LOAD_OP_LOAD;
    this->_format = this->_physicalDevice->getDepthFormat();
    this->_samples = this->_physicalDevice->getMsaaSamples();

    return *this;
}

AttachmentBuilder &AttachmentBuilder::defaultResolveAttachment() {
    this->_initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
    this->_finalLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;
    this->_loadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
    this->_format = this->_physicalDevice->getColorFormat();
    this->_samples = VK_SAMPLE_COUNT_1_BIT;

    return *this;
}

VkAttachmentDescription AttachmentBuilder::build() {
    return {
            .flags = 0,
            .format = this->_format.value_or(this->_physicalDevice->getColorFormat()),
            .samples = this->_samples.value_or(VK_SAMPLE_COUNT_1_BIT),
            .loadOp = this->_loadOp.value_or(VK_ATTACHMENT_LOAD_OP_DONT_CARE),
            .storeOp = VK_ATTACHMENT_STORE_OP_STORE,
            .stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE,
            .stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE,
            .initialLayout = this->_initialLayout.value_or(VK_IMAGE_LAYOUT_UNDEFINED),
            .finalLayout = this->_finalLayout.value_or(VK_IMAGE_LAYOUT_PRESENT_SRC_KHR)
    };
}
