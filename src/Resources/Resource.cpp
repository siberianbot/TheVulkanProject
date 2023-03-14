#include "Resource.hpp"

Resource::Resource(const std::vector<std::filesystem::path> &paths)
        : _paths(paths) {
    //
}

void Resource::load() {
    this->_isLoaded = true;
}

void Resource::unload() {
    this->_isLoaded = false;
}
