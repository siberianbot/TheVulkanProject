#include "RenderingResourceManager.hpp"

#include <fmt/format.h>

#include "src/Engine/EngineError.hpp"
#include "src/Engine/Log.hpp"
#include "src/Events/EventQueue.hpp"
#include "src/Rendering/RenderingObjectsAllocator.hpp"
#include "src/Rendering/Builders/ImageViewObjectBuilder.hpp"
#include "src/Rendering/Objects/BufferObject.hpp"
#include "src/Rendering/Objects/ImageObject.hpp"
#include "src/Rendering/Objects/ImageViewObject.hpp"
#include "src/Resources/ResourceDatabase.hpp"
#include "src/Resources/ResourceLoader.hpp"
#include "src/Resources/Readers/ImageReader.hpp"
#include "src/Resources/Readers/MeshReader.hpp"

static constexpr const char *RENDERING_RESOURCE_MANAGER = "RenderingResourceManager";

MeshRenderingData RenderingResourceManager::getMesh(const ResourceId &id) {
    auto it = this->_meshes.find(id);

    if (it != this->_meshes.end()) {
        return it->second;
    }

    return this->loadMesh(id);
}

TextureRenderingData RenderingResourceManager::getTexture(const ResourceId &id) {
    auto it = this->_textures.find(id);

    if (it != this->_textures.end()) {
        return it->second;
    }

    return this->loadTexture(id);
}

MeshRenderingData RenderingResourceManager::loadMesh(const ResourceId &id) {
    auto exception = [&id]() -> EngineError {
        return EngineError(fmt::format("Failed to upload mesh {0}", id));
    };

    auto resource = this->_resourceDatabase->tryGetResource(id);

    if (!resource.has_value()) {
        throw exception();
    }

    auto resourceData = this->_resourceLoader->tryLoad(resource.value());

    if (!resourceData.has_value()) {
        throw exception();
    }

    auto meshData = this->_meshReader->tryRead(resourceData.value());

    if (!meshData.has_value()) {
        throw exception();
    }

    MeshRenderingData mesh = {
            .vertexBuffer = this->_renderingObjectsAllocator->uploadVertices(meshData.value()->vertices),
            .indexBuffer = this->_renderingObjectsAllocator->uploadIndices(meshData.value()->indices)
    };

    this->_meshes[id] = mesh;

    return mesh;
}

TextureRenderingData RenderingResourceManager::loadTexture(const ResourceId &id) {
    auto exception = [&id]() -> EngineError {
        return EngineError(fmt::format("Failed to upload image {0}", id));
    };

    auto resource = this->_resourceDatabase->tryGetResource(id);

    if (!resource.has_value()) {
        throw exception();
    }

    auto resourceData = this->_resourceLoader->tryLoad(resource.value());

    if (!resourceData.has_value()) {
        throw exception();
    }

    auto imageData = this->_imageReader->tryRead(resourceData.value());

    if (!imageData.has_value()) {
        throw exception();
    }

    auto image = this->_renderingObjectsAllocator->uploadImage(imageData.value()->width,
                                                               imageData.value()->height,
                                                               imageData.value()->size(),
                                                               imageData.value()->image);

    auto imageView = ImageViewObjectBuilder(this->_vulkanObjectsAllocator)
            .fromImageObject(image)
            .withAspectFlags(VK_IMAGE_ASPECT_COLOR_BIT)
            .build();

    TextureRenderingData texture = {
            .image = image,
            .imageView = imageView
    };

    this->_textures[id] = texture;

    return texture;
}

void RenderingResourceManager::free(const ResourceId &id) {
    auto meshIterator = this->_meshes.find(id);

    if (meshIterator != this->_meshes.end()) {
        meshIterator->second.free();
        return;
    }

    auto textureIterator = this->_textures.find(id);

    if (textureIterator != this->_textures.end()) {
        textureIterator->second.free();
        return;
    }
}

RenderingResourceManager::RenderingResourceManager(
        const std::shared_ptr<Log> &log,
        const std::shared_ptr<EventQueue> &eventQueue,
        const std::shared_ptr<RenderingObjectsAllocator> &renderingObjectsAllocator,
        const std::shared_ptr<VulkanObjectsAllocator> &vulkanObjectsAllocator,
        const std::shared_ptr<ResourceDatabase> &resourceDatabase,
        const std::shared_ptr<ResourceLoader> &resourceLoader,
        const std::shared_ptr<ImageReader> &imageReader,
        const std::shared_ptr<MeshReader> &meshReader)
        : _log(log),
          _eventQueue(eventQueue),
          _renderingObjectsAllocator(renderingObjectsAllocator),
          _vulkanObjectsAllocator(vulkanObjectsAllocator),
          _resourceDatabase(resourceDatabase),
          _resourceLoader(resourceLoader),
          _imageReader(imageReader),
          _meshReader(meshReader) {
    this->_eventQueue->addHandler([this](const Event &event) {
        if (event.type != REPLACED_RESOURCE_EVENT &&
            event.type != REMOVED_RESOURCE_EVENT) {
            return;
        }

        this->free(event.resourceId());
    });
}

std::optional<MeshRenderingData> RenderingResourceManager::tryGetMesh(const ResourceId &id) {
    try {
        return this->getMesh(id);
    } catch (const std::exception &error) {
        this->_log->error(RENDERING_RESOURCE_MANAGER, error);
        return std::nullopt;
    }
}

std::optional<TextureRenderingData> RenderingResourceManager::tryGetTexture(const ResourceId &id) {
    try {
        return this->getTexture(id);
    } catch (const std::exception &error) {
        this->_log->error(RENDERING_RESOURCE_MANAGER, error);
        return std::nullopt;
    }
}
