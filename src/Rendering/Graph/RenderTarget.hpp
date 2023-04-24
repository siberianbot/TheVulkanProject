#ifndef RENDERING_GRAPH_RENDERTARGET_HPP
#define RENDERING_GRAPH_RENDERTARGET_HPP

#include <vulkan/vulkan.hpp>

#include "src/Rendering/Graph/RenderPassRef.hpp"
#include "src/Rendering/Graph/RenderTargetRef.hpp"

enum RenderTargetType {
    INPUT_RENDER_TARGET_TYPE = 1 << 0,
    COLOR_RENDER_TARGET_TYPE = 1 << 1,
    DEPTH_STENCIL_RENDER_TARGET_TYPE = 1 << 2
};

enum RenderTargetSource {
    IMAGE_RENDER_TARGET_SOURCE,
    SWAPCHAIN_RENDER_TARGET_SOURCE
};

enum RenderTargetFormat {
    DEFAULT_COLOR_RENDER_TARGET_FORMAT,
    DEFAULT_DEPTH_RENDER_TARGET_FORMAT,
};

struct RenderTargetInput {
    RenderPassRef passRef;
    vk::AttachmentLoadOp loadOp;
    vk::AttachmentStoreOp storeOp;
    vk::ImageLayout initialLayout;
    vk::ImageLayout finalLayout;
};

struct RenderTarget {
    RenderTargetInput input;
    RenderTargetType type;
    RenderTargetSource source;
    RenderTargetFormat format;
    vk::ClearValue clearValue;
};

#endif // RENDERING_GRAPH_RENDERTARGET_HPP
