#include "FramebuffersBuilder.hpp"

#include "src/Rendering/RenderingDevice.hpp"
#include "src/Rendering/Swapchain.hpp"
#include "src/Rendering/Objects/ImageViewObject.hpp"

FramebuffersBuilder::FramebuffersBuilder(RenderingDevice *renderingDevice,
                                         Swapchain *swapchain,
                                         VkRenderPass renderpass)
        : _renderingDevice(renderingDevice),
          _swapchain(swapchain),
          _renderpass(renderpass) {
    //
}

FramebuffersBuilder &FramebuffersBuilder::addAttachment(VkImageView imageView) {
    this->_attachments.push_back(imageView);

    return *this;
}

std::vector<VkFramebuffer> FramebuffersBuilder::build() {
    uint32_t count = this->_swapchain->getImageCount();
    std::vector<VkFramebuffer> framebuffers(count);

    for (uint32_t idx = 0; idx < count; idx++) {
        std::vector<VkImageView> attachments(this->_attachments.size());
        std::copy(this->_attachments.begin(), this->_attachments.end(), attachments.begin());

        attachments.push_back(this->_swapchain->getSwapchainImageView(idx)->getHandle());

        framebuffers[idx] = this->_renderingDevice->createFramebuffer(this->_renderpass,
                                                                      this->_swapchain->getSwapchainExtent(),
                                                                      attachments);
    }

    return framebuffers;
}
