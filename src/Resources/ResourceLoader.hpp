#ifndef RESOURCES_RESOURCELOADER_HPP
#define RESOURCES_RESOURCELOADER_HPP

#include <map>
#include <memory>
#include <optional>

#include "src/Resources/ResourceId.hpp"
#include "src/Resources/ResourceType.hpp"

class Log;
class EventQueue;
class Resource;
class ResourceData;

class ResourceLoader {
private:
    std::shared_ptr<Log> _log;
    std::shared_ptr<EventQueue> _eventQueue;

    std::map<ResourceId, std::shared_ptr<ResourceData>> _loadedResources;

    std::shared_ptr<ResourceData> loadResource(const std::weak_ptr<Resource> &resource);
    void setResource(const ResourceId &id, const std::shared_ptr<ResourceData> &resourceData);

    void freeResource(const std::map<ResourceId, std::shared_ptr<ResourceData>>::const_iterator &it);

public:
    ResourceLoader(const std::shared_ptr<Log> &log,
                   const std::shared_ptr<EventQueue> &eventQueue);

    [[nodiscard]] std::weak_ptr<ResourceData> load(const std::weak_ptr<Resource> &resource);
    [[nodiscard]] std::optional<std::weak_ptr<ResourceData>> tryLoad(const std::weak_ptr<Resource> &resource);

    void freeAll();
    void freeResource(const ResourceId &id);

    [[nodiscard]] bool isLoaded(const ResourceId &id) const;

    [[nodiscard]] const std::map<ResourceId, std::shared_ptr<ResourceData>> &loadedResources() const {
        return this->_loadedResources;
    }
};

#endif // RESOURCES_RESOURCELOADER_HPP
