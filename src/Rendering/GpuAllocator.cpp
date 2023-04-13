#include "GpuAllocator.hpp"

#include "src/Engine/EngineError.hpp"
#include "src/Engine/Log.hpp"
#include "src/Rendering/Proxies/LogicalDeviceProxy.hpp"
#include "src/Rendering/Proxies/PhysicalDeviceProxy.hpp"

static constexpr const char *GPU_ALLOCATOR_TAG = "GpuAllocator";

vk::DeviceMemory GpuAllocator::allocateMemory(const vk::MemoryRequirements &requirements,
                                              vk::MemoryPropertyFlags properties) {
    auto memoryProperties = this->_physicalDevice->getHandle().getMemoryProperties();

    std::optional<uint32_t> memTypeIdx;

    for (uint32_t idx = 0; idx < memoryProperties.memoryTypeCount; idx++) {
        bool typeMatches = requirements.memoryTypeBits & (1 << idx);
        bool propertiesMatches = (memoryProperties.memoryTypes[idx].propertyFlags & properties) == properties;

        if (!typeMatches || !propertiesMatches) {
            continue;
        }

        memTypeIdx = idx;
        break;
    }

    if (!memTypeIdx.has_value()) {
        throw EngineError("No memory type available for required allocation");
    }

    auto memoryAllocateInfo = vk::MemoryAllocateInfo()
            .setAllocationSize(requirements.size)
            .setMemoryTypeIndex(memTypeIdx.value());

    try {
        return this->_logicalDevice->getHandle().allocateMemory(memoryAllocateInfo);
    } catch (const std::exception &error) {
        this->_log->error(GPU_ALLOCATOR_TAG, error);
        throw EngineError("Failed to allocate memory");
    }
}

GpuAllocator::BufferAllocation GpuAllocator::createBufferAllocation(const BufferRequirements &requirements) {
    BufferAllocation allocation = {
            .requirements = requirements
    };

    auto bufferCreateInfo = vk::BufferCreateInfo()
            .setSharingMode(vk::SharingMode::eExclusive)
            .setSize(requirements.size)
            .setUsage(requirements.usage);

    try {
        allocation.buffer = this->_logicalDevice->getHandle().createBuffer(bufferCreateInfo);
    } catch (const std::exception &error) {
        this->_log->error(GPU_ALLOCATOR_TAG, error);
        throw EngineError("Failed to create buffer");
    }

    allocation.memory = this->allocateMemory(
            this->_logicalDevice->getHandle().getBufferMemoryRequirements(allocation.buffer),
            requirements.memoryProperties);

    try {
        this->_logicalDevice->getHandle().bindBufferMemory(allocation.buffer, allocation.memory, 0);
    } catch (const std::exception &error) {
        this->_log->error(GPU_ALLOCATOR_TAG, error);
        throw EngineError("Failed to bind memory to buffer");
    }

    this->_buffers.push_back(allocation);

    return allocation;
}

void GpuAllocator::freeBufferAllocation(const GpuAllocator::BufferAllocation &allocation) {
    this->_logicalDevice->getHandle().unmapMemory(allocation.memory);
    this->_logicalDevice->getHandle().free(allocation.memory);
    this->_logicalDevice->getHandle().destroy(allocation.buffer);
}

GpuAllocator::ImageAllocation GpuAllocator::createImageAllocation(const ImageRequirements &requirements) {
    ImageAllocation allocation = {
            .requirements = requirements
    };

    auto imageCreateInfo = vk::ImageCreateInfo()
            .setSharingMode(vk::SharingMode::eExclusive)
            .setFormat(requirements.format)
            .setExtent(requirements.extent)
            .setMipLevels(1)
            .setArrayLayers(requirements.layerCount.value_or(1))
            .setSamples(requirements.samples.value_or(vk::SampleCountFlagBits::e1))
            .setFlags(requirements.imageFlags.value_or(static_cast<vk::ImageCreateFlags>(0)));

    try {
        allocation.image = this->_logicalDevice->getHandle().createImage(imageCreateInfo);
    } catch (const std::exception &error) {
        this->_log->error(GPU_ALLOCATOR_TAG, error);
        throw EngineError("Failed to create image");
    }

    allocation.memory = this->allocateMemory(
            this->_logicalDevice->getHandle().getImageMemoryRequirements(allocation.image),
            requirements.memoryProperties);

    try {
        this->_logicalDevice->getHandle().bindImageMemory(allocation.image, allocation.memory, 0);
    } catch (const std::exception &error) {
        this->_log->error(GPU_ALLOCATOR_TAG, error);
        throw EngineError("Failed to bind memory to image");
    }

    auto imageSubresourceRange = vk::ImageSubresourceRange()
            .setAspectMask(requirements.aspectMask.value_or(vk::ImageAspectFlagBits::eNone))
            .setBaseMipLevel(0)
            .setLevelCount(1)
            .setBaseArrayLayer(0)
            .setLayerCount(requirements.layerCount.value_or(1));

    auto imageViewCreateInfo = vk::ImageViewCreateInfo()
            .setImage(allocation.image)
            .setFormat(requirements.format)
            .setViewType(requirements.type.value_or(vk::ImageViewType::e2D))
            .setSubresourceRange(imageSubresourceRange);

    try {
        allocation.imageView = this->_logicalDevice->getHandle().createImageView(imageViewCreateInfo);
    } catch (const std::exception &error) {
        this->_log->error(GPU_ALLOCATOR_TAG, error);
        throw EngineError("Failed to create image view");
    }

    this->_images.push_back(allocation);

    return allocation;
}

void GpuAllocator::freeImageAllocation(const GpuAllocator::ImageAllocation &allocation) {
    this->_logicalDevice->getHandle().destroy(allocation.imageView);
    this->_logicalDevice->getHandle().free(allocation.memory);
    this->_logicalDevice->getHandle().destroy(allocation.image);
}

std::shared_ptr<BufferView> GpuAllocator::createBufferView(const GpuAllocator::BufferAllocation &allocation) {
    auto view = std::make_shared<BufferView>();
    view->buffer = allocation.buffer;
    view->size = allocation.requirements.size;
    view->offset = 0;

    this->_bufferViews.push_back(view);

    return view;
}

std::shared_ptr<ImageView> GpuAllocator::createImageView(const GpuAllocator::ImageAllocation &allocation) {
    auto view = std::make_shared<ImageView>();
    view->image = allocation.image;
    view->imageView = allocation.imageView;

    this->_imageViews.push_back(view);

    return view;
}

GpuAllocator::GpuAllocator(const std::shared_ptr<Log> &log,
                           const std::shared_ptr<PhysicalDeviceProxy> &physicalDevice,
                           const std::shared_ptr<LogicalDeviceProxy> &logicalDevice)
        : _log(log),
          _physicalDevice(physicalDevice),
          _logicalDevice(logicalDevice) {
    //
}

std::weak_ptr<BufferView> GpuAllocator::allocateBuffer(const BufferRequirements &requirements, bool map) {
    auto allocation = this->createBufferAllocation(requirements);
    auto view = this->createBufferView(allocation);

    if (map) {
        try {
            view->ptr = this->_logicalDevice->getHandle().mapMemory(allocation.memory, view->offset, view->size);
        } catch (const std::exception &error) {
            this->_log->error(GPU_ALLOCATOR_TAG, error);
            throw EngineError("Failed to map memory");
        }
    }

    return view;
}

std::weak_ptr<ImageView> GpuAllocator::allocateImage(const ImageRequirements &requirements) {
    auto allocation = this->createImageAllocation(requirements);
    return this->createImageView(allocation);
}

void GpuAllocator::freeBuffer(const std::weak_ptr<BufferView> &bufferView) {
    if (bufferView.expired()) {
        this->_log->warning(GPU_ALLOCATOR_TAG, "Attempt to free expired buffer");
        return;
    }

    auto lockedBufferView = bufferView.lock();

    auto viewIt = std::find_if(this->_bufferViews.begin(), this->_bufferViews.end(),
                               [&lockedBufferView](const std::shared_ptr<BufferView> view) {
                                   return view->buffer == lockedBufferView->buffer;
                               });

    if (viewIt == this->_bufferViews.end()) {
        this->_log->error(GPU_ALLOCATOR_TAG, "Attempt to free unknown buffer");
        return;
    }

    this->_bufferViews.erase(viewIt);

    auto bufferIt = std::find_if(this->_buffers.begin(), this->_buffers.end(),
                                 [&lockedBufferView](const BufferAllocation &allocation) {
                                     return allocation.buffer == lockedBufferView->buffer;
                                 });

    if (bufferIt == this->_buffers.end()) {
        this->_log->error(GPU_ALLOCATOR_TAG, "Attempt to free unknown buffer");
        return;
    }

    this->freeBufferAllocation(*bufferIt);
    this->_buffers.erase(bufferIt);
}

void GpuAllocator::freeImage(const std::weak_ptr<ImageView> &imageView) {
    if (imageView.expired()) {
        this->_log->warning(GPU_ALLOCATOR_TAG, "Attempt to free expired image");
        return;
    }

    auto lockedImageView = imageView.lock();

    auto viewIt = std::find_if(this->_imageViews.begin(), this->_imageViews.end(),
                               [&lockedImageView](const std::shared_ptr<ImageView> view) {
                                   return view->image == lockedImageView->image;
                               });

    if (viewIt == this->_imageViews.end()) {
        this->_log->error(GPU_ALLOCATOR_TAG, "Attempt to free unknown image");
        return;
    }

    this->_imageViews.erase(viewIt);

    auto imageIt = std::find_if(this->_images.begin(), this->_images.end(),
                                [&lockedImageView](const ImageAllocation &allocation) {
                                    return allocation.image == lockedImageView->image;
                                });

    if (imageIt == this->_images.end()) {
        this->_log->error(GPU_ALLOCATOR_TAG, "Attempt to free unknown image");
        return;
    }

    this->freeImageAllocation(*imageIt);
    this->_images.erase(imageIt);
}

void GpuAllocator::freeAll() {
    this->_bufferViews.clear();
    this->_imageViews.clear();

    for (const BufferAllocation &allocation: this->_buffers) {
        this->freeBufferAllocation(allocation);
    }

    for (const ImageAllocation &allocation: this->_images) {
        this->freeImageAllocation(allocation);
    }

    this->_buffers.clear();
    this->_images.clear();
}
