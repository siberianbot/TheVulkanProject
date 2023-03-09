#include "SpecializationInfoBuilder.hpp"

SpecializationInfoBuilder &SpecializationInfoBuilder::withEntry(uint32_t id, uint32_t offset, size_t size) {
    this->_entries.push_back(VkSpecializationMapEntry{
            .constantID = id,
            .offset = offset,
            .size = size
    });

    return *this;
}

SpecializationInfoBuilder &SpecializationInfoBuilder::withSize(size_t size) {
    this->_size = size;

    return *this;
}

SpecializationInfoBuilder &SpecializationInfoBuilder::withData(void *ptr) {
    this->_ptr = ptr;

    return *this;
}

VkSpecializationInfo SpecializationInfoBuilder::build() {
    if (!this->_size.has_value() || !this->_ptr.has_value() || this->_ptr == nullptr) {
        throw std::runtime_error("Size and pointer to data are required");
    }

    return VkSpecializationInfo{
            .mapEntryCount = static_cast<uint32_t>(this->_entries.size()),
            .pMapEntries = this->_entries.data(),
            .dataSize = this->_size.value(),
            .pData = this->_ptr.value()
    };
}