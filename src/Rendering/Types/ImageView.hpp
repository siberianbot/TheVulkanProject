#ifndef RENDERING_TYPES_IMAGEVIEW_HPP
#define RENDERING_TYPES_IMAGEVIEW_HPP

#include <vulkan/vulkan.hpp>

struct ImageView {
    vk::Image image;
    vk::ImageView imageView;
};

#endif // RENDERING_TYPES_IMAGEVIEW_HPP
