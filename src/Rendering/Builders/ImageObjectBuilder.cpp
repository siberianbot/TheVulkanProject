#include "ImageObjectBuilder.hpp"

#include "src/Rendering/Common.hpp"
#include "src/Rendering/RenderingDevice.hpp"
#include "src/Rendering/VulkanObjectsAllocator.hpp"
#include "src/Rendering/Objects/ImageObject.hpp"

ImageObjectBuilder::ImageObjectBuilder(const std::shared_ptr<RenderingDevice> &renderingDevice,
                                       const std::shared_ptr<VulkanObjectsAllocator> &vulkanObjectsAllocator)
        : _renderingDevice(renderingDevice),
          _vulkanObjectsAllocator(vulkanObjectsAllocator) {
    //
}

ImageObjectBuilder &ImageObjectBuilder::withExtent(uint32_t width, uint32_t height) {
    this->_extent.width = width;
    this->_extent.height = height;
    this->_extent.depth = 1;

    return *this;
}

ImageObjectBuilder &ImageObjectBuilder::withFormat(VkFormat format) {
    this->_format = format;

    return *this;
}

ImageObjectBuilder &ImageObjectBuilder::withSamples(VkSampleCountFlagBits samples) {
    this->_samples = samples;

    return *this;
}

ImageObjectBuilder &ImageObjectBuilder::withUsage(VkImageUsageFlags usage) {
    this->_usage = this->_usage.value_or(0) | usage;

    return *this;
}

ImageObjectBuilder &ImageObjectBuilder::withInitialLayout(VkImageLayout initialLayout) {
    this->_initialLayout = initialLayout;

    return *this;
}

ImageObjectBuilder &ImageObjectBuilder::cubeCompatible() {
    this->_flags = this->_flags.value_or(0) | VK_IMAGE_CREATE_CUBE_COMPATIBLE_BIT;
    this->_count = 6;

    return *this;
}

ImageObjectBuilder &ImageObjectBuilder::arrayCompatible(uint32_t count) {
    this->_count = count;

    return *this;
}

std::shared_ptr<ImageObject> ImageObjectBuilder::build() {
    VkImageCreateInfo imageCreateInfo = {
            .sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO,
            .pNext = nullptr,
            .flags = this->_flags.value_or(0),
            .imageType = VK_IMAGE_TYPE_2D,
            .format = this->_format,
            .extent = this->_extent,
            .mipLevels = 1,
            .arrayLayers = this->_count.value_or(1),
            .samples = this->_samples.value_or(VK_SAMPLE_COUNT_1_BIT),
            .tiling = VK_IMAGE_TILING_OPTIMAL,
            .usage = this->_usage.value_or(0),
            .sharingMode = VK_SHARING_MODE_EXCLUSIVE,
            .queueFamilyIndexCount = 0,
            .pQueueFamilyIndices = nullptr,
            .initialLayout = this->_initialLayout.value_or(VK_IMAGE_LAYOUT_UNDEFINED),
    };

    VkImage image = this->_vulkanObjectsAllocator->createImage(&imageCreateInfo);
    VkDeviceMemory memory = this->_vulkanObjectsAllocator->allocateMemoryForImage(image,
                                                                                  VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);

    vkEnsure(vkBindImageMemory(this->_renderingDevice->getHandle(), image, memory, 0));

    return std::make_shared<ImageObject>(this->_vulkanObjectsAllocator, image, memory, imageCreateInfo.arrayLayers,
                                         this->_format);
}
