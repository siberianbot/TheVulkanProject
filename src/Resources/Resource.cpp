#include "Resource.hpp"

Resource::Resource(const ResourceId &id,
                   const ResourceType &type,
                   const std::filesystem::path &path)
        : _id(id),
          _type(type),
          _path(path) {
    //
}
