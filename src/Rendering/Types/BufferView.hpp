#ifndef RENDERING_TYPES_BUFFERVIEW_HPP
#define RENDERING_TYPES_BUFFERVIEW_HPP

#include <vulkan/vulkan.hpp>

struct BufferView {
    vk::Buffer buffer;
    vk::DeviceSize offset;
    vk::DeviceSize size;
    std::optional<void *> ptr;
};

#endif // RENDERING_TYPES_BUFFERVIEW_HPP
