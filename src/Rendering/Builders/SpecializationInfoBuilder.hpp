#ifndef RENDERING_BUILDERS_SPECIALIZATIONINFBUILDER_HPP
#define RENDERING_BUILDERS_SPECIALIZATIONINFBUILDER_HPP

#include <optional>
#include <vector>

#include <vulkan/vulkan.hpp>

class SpecializationInfoBuilder {
private:
    std::vector<VkSpecializationMapEntry> _entries;
    std::optional<size_t> _size;
    std::optional<void *> _ptr;

public:
    SpecializationInfoBuilder &withEntry(uint32_t id, uint32_t offset, size_t size);
    SpecializationInfoBuilder &withSize(size_t size);
    SpecializationInfoBuilder &withData(void *ptr);

    VkSpecializationInfo build();
};

#endif // RENDERING_BUILDERS_SPECIALIZATIONINFBUILDER_HPP
