#include "Resource.hpp"

Resource::Resource(const ResourceId &id,
                   const ResourceType &type,
                   const std::vector<std::filesystem::path> &paths)
        : _id(id),
          _type(type),
          _paths(paths) {
    //
}
