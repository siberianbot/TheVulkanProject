#ifndef RENDERING_GRAPH_RENDERTARGET_HPP
#define RENDERING_GRAPH_RENDERTARGET_HPP

#include <string>

using RenderTargetRef = std::string;

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

struct RenderTarget {
    RenderTargetType type;
    RenderTargetSource source;
    RenderTargetFormat format;
};

#endif // RENDERING_GRAPH_RENDERTARGET_HPP
