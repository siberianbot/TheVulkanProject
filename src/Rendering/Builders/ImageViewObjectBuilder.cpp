#include "ImageViewObjectBuilder.hpp"

#include "src/Rendering/VulkanObjectsAllocator.hpp"
#include "src/Rendering/Objects/ImageObject.hpp"
#include "src/Rendering/Objects/ImageViewObject.hpp"

ImageViewObjectBuilder::ImageViewObjectBuilder(const std::shared_ptr<VulkanObjectsAllocator> &vulkanObjectsAllocator)
        : _vulkanObjectsAllocator(vulkanObjectsAllocator) {
    //
}

ImageViewObjectBuilder &ImageViewObjectBuilder::fromImageObject(const std::shared_ptr<ImageObject> &imageObject) {
    this->_image = imageObject->getHandle();
    this->_format = imageObject->getFormat();

    return *this;
}

ImageViewObjectBuilder &ImageViewObjectBuilder::withImage(VkImage image) {
    this->_image = image;

    return *this;
}

ImageViewObjectBuilder &ImageViewObjectBuilder::withFormat(VkFormat format) {
    this->_format = format;

    return *this;
}

ImageViewObjectBuilder &ImageViewObjectBuilder::withType(VkImageViewType type) {
    this->_type = type;

    return *this;
}

ImageViewObjectBuilder &ImageViewObjectBuilder::withAspectFlags(VkImageAspectFlags aspectMask) {
    this->_aspectMask = aspectMask;

    return *this;
}

ImageViewObjectBuilder &ImageViewObjectBuilder::withLayers(uint32_t baseLayer, uint32_t count) {
    this->_baseArrayLayer = baseLayer;
    this->_layerCount = count;

    return *this;
}

std::shared_ptr<ImageViewObject> ImageViewObjectBuilder::build() {
    VkImageViewCreateInfo createInfo = {
            .sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO,
            .pNext = nullptr,
            .flags = 0,
            .image = this->_image,
            .viewType = this->_type.value_or(VK_IMAGE_VIEW_TYPE_2D),
            .format = this->_format,
            .components = {},
            .subresourceRange = {
                    .aspectMask = this->_aspectMask.value_or(VK_IMAGE_ASPECT_NONE),
                    .baseMipLevel = 0,
                    .levelCount = 1,
                    .baseArrayLayer = this->_baseArrayLayer.value_or(0),
                    .layerCount = this->_layerCount.value_or(1)
            }
    };

    VkImageView imageView = this->_vulkanObjectsAllocator->createImageView(&createInfo);

    return std::make_shared<ImageViewObject>(this->_vulkanObjectsAllocator, this->_image, imageView);
}
