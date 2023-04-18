#ifndef RENDERING_RENDERER_HPP
#define RENDERING_RENDERER_HPP

#include <memory>
#include <thread>
#include <vector>

#include <vulkan/vulkan.hpp>

class Log;
class GpuManager;
class Swapchain;
class CommandBufferProxy;
class LogicalDeviceProxy;
class Window;

// TODO: not use explicitly
class DebugUIRenderStage;

class Renderer {
private:
    struct FrameSync {
        vk::Fence fence;
        vk::Semaphore imageAvailableSemaphore;
        vk::Semaphore renderFinishedSemaphore;
    };

    std::shared_ptr<Log> _log;
    std::shared_ptr<GpuManager> _gpuManager;
    std::shared_ptr<Window> _window;

    std::shared_ptr<LogicalDeviceProxy> _logicalDevice;

    uint32_t _inflightFrameCount;
    uint32_t _currentFrameIdx;
    std::vector<FrameSync> _frameSyncs;
    std::vector<std::shared_ptr<CommandBufferProxy>> _commandBuffers;
    std::shared_ptr<Swapchain> _swapchain;
    std::jthread _renderThread;

    // TODO: not use explicitly
    std::unique_ptr<DebugUIRenderStage> _renderStage;

    void render();

public:
    Renderer(const std::shared_ptr<Log> &log,
             const std::shared_ptr<GpuManager> &gpuManager,
             const std::shared_ptr<Window> &window);

    void init();
    void destroy();
};

#endif // RENDERING_RENDERER_HPP
