#include "ResourceLoader.hpp"

#include <fstream>

#include <fmt/format.h>

#include "src/Engine/EngineError.hpp"
#include "src/Engine/Log.hpp"
#include "src/Events/EventQueue.hpp"
#include "src/Resources/Resource.hpp"
#include "src/Resources/ResourceData.hpp"

static constexpr const char *RESOURCE_LOADER_TAG = "ResourceLoader";

std::shared_ptr<ResourceData> ResourceLoader::loadResource(const std::weak_ptr<Resource> &resource) {
    if (resource.expired()) {
        throw EngineError("Resource is expired");
    }

    std::shared_ptr<Resource> lockedResource = resource.lock();

    std::ifstream stream = std::ifstream(lockedResource->path(), std::ios::ate | std::ios::binary);

    if (!stream.is_open()) {
        throw EngineError(fmt::format("Failed to load resource {0}: stream is not available", lockedResource->id()));
    }

    size_t size = stream.tellg();
    DataBuffer data(size);

    stream.seekg(0);
    stream.read(data.data(), size);

    std::shared_ptr<ResourceData> instance = std::make_shared<ResourceData>(lockedResource->id(), data);
    this->setResource(lockedResource->id(), instance);

    this->_eventQueue->pushEvent({.type = LOADED_RESOURCE_EVENT, .value = lockedResource->id()});
    this->_log->info(RESOURCE_LOADER_TAG, fmt::format("Loaded resource {0}", lockedResource->id()));

    return instance;
}

void ResourceLoader::setResource(const ResourceId &id, const std::shared_ptr<ResourceData> &resourceData) {
    auto it = this->_loadedResources.find(id);

    if (it != this->_loadedResources.end()) {
        this->_log->info(RESOURCE_LOADER_TAG,
                         fmt::format("Resource {0} has loaded data, previous data will be unloaded", id));
        this->_eventQueue->pushEvent({.type = UNLOADED_RESOURCE_EVENT, .value = id});

        it->second = resourceData;
    } else {
        this->_loadedResources[id] = resourceData;
    }
}

void ResourceLoader::freeResource(const std::map<ResourceId, std::shared_ptr<ResourceData>>::const_iterator &it) {
    ResourceId id = it->first;

    this->_loadedResources.erase(it);

    this->_eventQueue->pushEvent({.type = UNLOADED_RESOURCE_EVENT, .value = id});
    this->_log->info(RESOURCE_LOADER_TAG, fmt::format("Unloaded resource {0}", id));
}

ResourceLoader::ResourceLoader(const std::shared_ptr<Log> &log,
                               const std::shared_ptr<EventQueue> &eventQueue)
        : _log(log),
          _eventQueue(eventQueue) {
    eventQueue->addHandler([this](const Event &event) {
        if (event.type != REPLACED_RESOURCE_EVENT &&
            event.type != REMOVED_RESOURCE_EVENT) {
            return;
        }

        auto it = this->_loadedResources.find(event.resourceId());

        if (it == this->_loadedResources.end()) {
            return;
        }

        this->freeResource(it);
    });
}

std::weak_ptr<ResourceData> ResourceLoader::load(const std::weak_ptr<Resource> &resource) {
    return loadResource(resource);
}

std::optional<std::weak_ptr<ResourceData>> ResourceLoader::tryLoad(const std::weak_ptr<Resource> &resource) {
    try {
        return loadResource(resource);
    } catch (const std::exception &error) {
        this->_log->error(RESOURCE_LOADER_TAG, error);
        return std::nullopt;
    }
}

void ResourceLoader::freeAll() {
    for (const auto &[id, instance]: this->_loadedResources) {
        this->_eventQueue->pushEvent({.type = UNLOADED_RESOURCE_EVENT, .value = id});
    }

    this->_loadedResources.clear();
}

void ResourceLoader::freeResource(const ResourceId &id) {
    auto it = this->_loadedResources.find(id);

    if (it == this->_loadedResources.end()) {
        this->_log->warning(RESOURCE_LOADER_TAG, fmt::format("Attempt to unload unloaded resource {0}", id));
        return;
    }

    this->freeResource(it);
}

bool ResourceLoader::isLoaded(const ResourceId &id) const {
    return this->_loadedResources.find(id) != this->_loadedResources.end();
}
