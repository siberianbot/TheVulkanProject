#ifndef RENDERING_TYPES_COMMAND_HPP
#define RENDERING_TYPES_COMMAND_HPP

#include <functional>

#include <vulkan/vulkan.hpp>

using Command = std::function<void(const vk::CommandBuffer &cmdBuffer)>;

#endif // RENDERING_TYPES_COMMAND_HPP
