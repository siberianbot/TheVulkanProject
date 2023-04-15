#include "GpuResourceManager.hpp"

#include <string_view>

#include <fmt/core.h>

#include "src/Engine/EngineError.hpp"
#include "src/Engine/Log.hpp"
#include "src/Events/EventQueue.hpp"
#include "src/Rendering/CommandManager.hpp"
#include "src/Rendering/GpuAllocator.hpp"
#include "src/Rendering/Proxies/CommandBufferProxy.hpp"
#include "src/Rendering/Proxies/LogicalDeviceProxy.hpp"
#include "src/Resources/Resource.hpp"
#include "src/Resources/ResourceDatabase.hpp"
#include "src/Resources/ResourceLoader.hpp"
#include "src/Resources/Readers/ImageReader.hpp"
#include "src/Resources/Readers/MeshReader.hpp"

static constexpr std::string_view GPU_RESOURCE_MANAGER_TAG = "GpuResourceManager";

template<typename T>
std::weak_ptr<BufferView> uploadBuffer(const std::shared_ptr<CommandManager> &commandManager,
                                       const std::shared_ptr<GpuAllocator> &allocator,
                                       const std::shared_ptr<LogicalDeviceProxy> &logicalDevice,
                                       const std::vector<T> &data,
                                       vk::BufferUsageFlags targetUsage) {
    vk::DeviceSize size = sizeof(T) * data.size();

    BufferRequirements stagingBufferRequirements = {
            .size = size,
            .usage = vk::BufferUsageFlagBits::eTransferSrc,
            .memoryProperties = vk::MemoryPropertyFlagBits::eHostVisible |
                                vk::MemoryPropertyFlagBits::eHostCoherent
    };

    auto stagingBufferView = allocator->allocateBuffer(stagingBufferRequirements, true).lock();

    BufferRequirements resultBufferRequirements = {
            .size = size,
            .usage = vk::BufferUsageFlagBits::eTransferDst | targetUsage,
            .memoryProperties = vk::MemoryPropertyFlagBits::eDeviceLocal
    };

    auto targetBufferView = allocator->allocateBuffer(resultBufferRequirements, true).lock();

    auto commandBuffer = commandManager->createPrimaryBuffer();

    auto beginInfo = vk::CommandBufferBeginInfo()
            .setFlags(vk::CommandBufferUsageFlagBits::eOneTimeSubmit);

    commandBuffer->getHandle().begin(beginInfo);

    auto bufferCopy = vk::BufferCopy()
            .setSize(size);

    commandBuffer->getHandle().copyBuffer(stagingBufferView->buffer,
                                          targetBufferView->buffer,
                                          {bufferCopy});

    commandBuffer->getHandle().end();

    auto commandBuffers = {commandBuffer->getHandle()};
    auto submit = vk::SubmitInfo()
            .setCommandBuffers(commandBuffers);

    logicalDevice->getGraphicsQueue().submit({submit});
    logicalDevice->getGraphicsQueue().waitIdle();

    commandBuffer->destroy();

    allocator->freeBuffer(stagingBufferView);

    return targetBufferView;
}

std::weak_ptr<ImageView> uploadImage(const std::shared_ptr<CommandManager> &commandManager,
                                     const std::shared_ptr<GpuAllocator> &allocator,
                                     const std::shared_ptr<LogicalDeviceProxy> &logicalDevice,
                                     const std::unique_ptr<ImageData> &imageData) {
    vk::DeviceSize size = imageData->size();
    vk::Extent3D extent = vk::Extent3D(imageData->width, imageData->height, 1);

    BufferRequirements stagingBufferRequirements = {
            .size = size,
            .usage = vk::BufferUsageFlagBits::eTransferSrc,
            .memoryProperties = vk::MemoryPropertyFlagBits::eHostVisible |
                                vk::MemoryPropertyFlagBits::eHostCoherent
    };

    auto stagingBufferView = allocator->allocateBuffer(stagingBufferRequirements, true).lock();

    ImageRequirements imageRequirements = {
            .size = size,
            .usage = vk::ImageUsageFlagBits::eTransferDst | vk::ImageUsageFlagBits::eSampled,
            .memoryProperties = vk::MemoryPropertyFlagBits::eDeviceLocal,
            .extent = extent,
            .format = vk::Format::eR8G8B8A8Srgb,
            .aspectMask = vk::ImageAspectFlagBits::eColor
    };

    auto imageView = allocator->allocateImage(imageRequirements).lock();

    auto commandBuffer = commandManager->createPrimaryBuffer();

    auto beginInfo = vk::CommandBufferBeginInfo()
            .setFlags(vk::CommandBufferUsageFlagBits::eOneTimeSubmit);

    commandBuffer->getHandle().begin(beginInfo);

    auto memoryBarrier = vk::ImageMemoryBarrier()
            .setImage(imageView->image)
            .setSubresourceRange(vk::ImageSubresourceRange()
                                         .setAspectMask(vk::ImageAspectFlagBits::eColor)
                                         .setLayerCount(1)
                                         .setLevelCount(1));

    memoryBarrier
            .setDstAccessMask(vk::AccessFlagBits::eTransferWrite)
            .setOldLayout(vk::ImageLayout::eUndefined)
            .setNewLayout(vk::ImageLayout::eTransferDstOptimal);

    commandBuffer->getHandle().pipelineBarrier(vk::PipelineStageFlagBits::eTopOfPipe,
                                               vk::PipelineStageFlagBits::eTransfer,
                                               vk::DependencyFlags(),
                                               {}, {}, {memoryBarrier});

    auto bufferImageCopy = vk::BufferImageCopy()
            .setBufferRowLength(imageData->width)
            .setBufferImageHeight(imageData->height)
            .setImageExtent(extent)
            .setImageSubresource(vk::ImageSubresourceLayers()
                                         .setAspectMask(vk::ImageAspectFlagBits::eColor)
                                         .setLayerCount(1));

    commandBuffer->getHandle().copyBufferToImage(stagingBufferView->buffer,
                                                 imageView->image,
                                                 vk::ImageLayout::eTransferDstOptimal,
                                                 {bufferImageCopy});

    memoryBarrier
            .setSrcAccessMask(vk::AccessFlagBits::eTransferWrite)
            .setDstAccessMask(vk::AccessFlagBits::eShaderRead)
            .setOldLayout(vk::ImageLayout::eTransferDstOptimal)
            .setNewLayout(vk::ImageLayout::eShaderReadOnlyOptimal);

    commandBuffer->getHandle().pipelineBarrier(vk::PipelineStageFlagBits::eTopOfPipe,
                                               vk::PipelineStageFlagBits::eFragmentShader,
                                               vk::DependencyFlags(),
                                               {}, {}, {memoryBarrier});

    commandBuffer->getHandle().end();

    auto commandBuffers = {commandBuffer->getHandle()};
    auto submit = vk::SubmitInfo()
            .setCommandBuffers(commandBuffers);

    logicalDevice->getGraphicsQueue().submit({submit});
    logicalDevice->getGraphicsQueue().waitIdle();

    commandBuffer->destroy();

    allocator->freeBuffer(stagingBufferView);

    return imageView;
}

std::weak_ptr<Mesh> GpuResourceManager::getMesh(const ResourceId &resourceId) {
    auto it = this->_meshes.find(resourceId);

    if (it != this->_meshes.end()) {
        return it->second;
    }

    std::shared_ptr<Mesh> mesh = this->loadMesh(resourceId);
    this->_meshes.emplace(resourceId, mesh);

    return mesh;
}

std::shared_ptr<Mesh> GpuResourceManager::loadMesh(const ResourceId &resourceId) {
    auto generalException = [&resourceId]() {
        return EngineError(fmt::format("Failed to load mesh {0}", resourceId));
    };

    auto resource = this->_resourceDatabase->tryGetResource(resourceId);

    if (!resource.has_value()) {
        throw generalException();
    }

    auto lockedResource = resource.value().lock();

    if (lockedResource->type() != MESH_RESOURCE) {
        throw EngineError(fmt::format("Resource {0} is not a mesh", resourceId));
    }

    auto resourceData = this->_resourceLoader->tryLoad(lockedResource);

    if (!resourceData.has_value()) {
        throw generalException();
    }

    auto meshData = this->_meshReader->tryRead(resourceData.value());

    if (!meshData.has_value()) {
        throw generalException();
    }

    this->_resourceLoader->freeResource(resourceId);

    auto mesh = std::make_shared<Mesh>();

    try {
        mesh->vertexBuffer = uploadBuffer(this->_commandManager, this->_allocator, this->_logicalDevice,
                                          meshData.value()->vertices, vk::BufferUsageFlagBits::eVertexBuffer);
        mesh->indexBuffer = uploadBuffer(this->_commandManager, this->_allocator, this->_logicalDevice,
                                         meshData.value()->indices, vk::BufferUsageFlagBits::eIndexBuffer);
    } catch (const std::exception &error) {
        this->_log->error(GPU_RESOURCE_MANAGER_TAG, error);
        throw generalException();
    }

    return mesh;
}

void GpuResourceManager::freeMesh(const std::shared_ptr<Mesh> &mesh) {
    this->_allocator->freeBuffer(mesh->vertexBuffer);
    this->_allocator->freeBuffer(mesh->indexBuffer);
}

std::weak_ptr<Texture> GpuResourceManager::getTexture(const ResourceId &resourceId) {
    auto it = this->_textures.find(resourceId);

    if (it != this->_textures.end()) {
        return it->second;
    }

    std::shared_ptr<Texture> texture = this->loadTexture(resourceId);
    this->_textures.emplace(resourceId, texture);

    return texture;
}

std::shared_ptr<Texture> GpuResourceManager::loadTexture(const ResourceId &resourceId) {
    auto generalException = [&resourceId]() {
        return EngineError(fmt::format("Failed to load image {0}", resourceId));
    };

    auto resource = this->_resourceDatabase->tryGetResource(resourceId);

    if (!resource.has_value()) {
        throw generalException();
    }

    auto lockedResource = resource.value().lock();

    if (lockedResource->type() != IMAGE_RESOURCE) {
        throw EngineError(fmt::format("Resource {0} is not an image", resourceId));
    }

    auto resourceData = this->_resourceLoader->tryLoad(lockedResource);

    if (!resourceData.has_value()) {
        throw generalException();
    }

    auto imageData = this->_imageReader->tryRead(resourceData.value());

    if (!imageData.has_value()) {
        throw generalException();
    }

    auto texture = std::make_shared<Texture>();

    try {
        texture->image = uploadImage(this->_commandManager, this->_allocator, this->_logicalDevice,
                                     imageData.value());
    } catch (const std::exception &error) {
        this->_log->error(GPU_RESOURCE_MANAGER_TAG, error);
        throw generalException();
    }

    return texture;
}

void GpuResourceManager::freeTexture(const std::shared_ptr<Texture> &texture) {
    this->_allocator->freeImage(texture->image);
}

GpuResourceManager::GpuResourceManager(const std::shared_ptr<Log> &log,
                                       const std::shared_ptr<EventQueue> &eventQueue,
                                       const std::shared_ptr<ResourceDatabase> resourceDatabase,
                                       const std::shared_ptr<ResourceLoader> resourceLoader,
                                       const std::shared_ptr<CommandManager> &commandManager,
                                       const std::shared_ptr<GpuAllocator> &allocator,
                                       const std::shared_ptr<LogicalDeviceProxy> &logicalDevice)
        : _log(log),
          _eventQueue(eventQueue),
          _resourceDatabase(resourceDatabase),
          _resourceLoader(resourceLoader),
          _commandManager(commandManager),
          _allocator(allocator),
          _logicalDevice(logicalDevice),
          _imageReader(std::make_shared<ImageReader>(this->_log)),
          _meshReader(std::make_shared<MeshReader>(this->_log)) {
    //
}

void GpuResourceManager::init() {
    this->_handlerIdx = this->_eventQueue->addHandler([this](const Event &event) {
        if (event.type != REPLACED_RESOURCE_EVENT && event.type != REMOVED_RESOURCE_EVENT) {
            return;
        }

        auto resourceId = std::get<ResourceId>(event.value);

        auto meshIt = this->_meshes.find(resourceId);

        if (meshIt != this->_meshes.end()) {
            this->freeMesh(meshIt->second);
            return;
        }
    });
}

void GpuResourceManager::destroy() {
    this->freeAll();

    this->_eventQueue->removeHandler(this->_handlerIdx);
}

std::optional<std::weak_ptr<Mesh>> GpuResourceManager::tryGetMesh(const ResourceId &resourceId) {
    try {
        return this->getMesh(resourceId);
    } catch (const std::exception &error) {
        this->_log->error(GPU_RESOURCE_MANAGER_TAG, error);

        return std::nullopt;
    }
}

std::optional<std::weak_ptr<Texture>> GpuResourceManager::tryGetTexture(const ResourceId &resourceId) {
    try {
        return this->getTexture(resourceId);
    } catch (const std::exception &error) {
        this->_log->error(GPU_RESOURCE_MANAGER_TAG, error);

        return std::nullopt;
    }
}

void GpuResourceManager::free(const ResourceId &resourceId) {
    auto meshIt = this->_meshes.find(resourceId);

    if (meshIt != this->_meshes.end()) {
        this->freeMesh(meshIt->second);
        return;
    }

    auto textureIt = this->_textures.find(resourceId);

    if (textureIt != this->_textures.end()) {
        this->freeTexture(textureIt->second);
        return;
    }

    this->_log->warning(GPU_RESOURCE_MANAGER_TAG, fmt::format("Attempt to free unknown resource {0}", resourceId));
}

void GpuResourceManager::freeAll() {
    for (const auto &[id, mesh]: this->_meshes) {
        this->freeMesh(mesh);
    }

    this->_meshes.clear();
}
