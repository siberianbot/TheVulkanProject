#include "BufferObjectBuilder.hpp"

#include "src/Rendering/Common.hpp"
#include "src/Rendering/RenderingDevice.hpp"
#include "src/Rendering/VulkanObjectsAllocator.hpp"
#include "src/Rendering/Objects/BufferObject.hpp"

BufferObjectBuilder::BufferObjectBuilder(const std::shared_ptr<RenderingDevice> &renderingDevice,
                                         const std::shared_ptr<VulkanObjectsAllocator> &vulkanObjectsAllocator)
        : _renderingDevice(renderingDevice),
          _vulkanObjectsAllocator(vulkanObjectsAllocator) {
    //
}

BufferObjectBuilder &BufferObjectBuilder::withSize(VkDeviceSize size) {
    this->_size = size;

    return *this;
}

BufferObjectBuilder &BufferObjectBuilder::withUsage(VkBufferUsageFlags usage) {
    this->_usage = usage;

    return *this;
}

BufferObjectBuilder &BufferObjectBuilder::hostAvailable() {
    this->_memoryProperties = VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT;

    return *this;
}

BufferObjectBuilder &BufferObjectBuilder::deviceLocal() {
    this->_memoryProperties = VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT;

    return *this;
}

std::shared_ptr<BufferObject> BufferObjectBuilder::build() {
    VkBufferCreateInfo bufferCreateInfo = {
            .sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO,
            .pNext = nullptr,
            .flags = 0,
            .size = this->_size.value_or(0),
            .usage = this->_usage.value_or(0),
            .sharingMode = VK_SHARING_MODE_EXCLUSIVE,
            .queueFamilyIndexCount = 0,
            .pQueueFamilyIndices = nullptr
    };

    VkBuffer buffer = this->_vulkanObjectsAllocator->createBuffer(&bufferCreateInfo);
    VkDeviceMemory memory = this->_vulkanObjectsAllocator->allocateMemoryForBuffer(
            buffer, this->_memoryProperties.value_or(VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT));

    vkEnsure(vkBindBufferMemory(this->_renderingDevice->getHandle(), buffer, memory, 0));

    return std::make_shared<BufferObject>(this->_renderingDevice, this->_vulkanObjectsAllocator, buffer, memory,
                                          this->_size.value_or(0));
}
