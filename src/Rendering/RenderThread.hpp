#ifndef RENDERING_RENDERTHREAD_HPP
#define RENDERING_RENDERTHREAD_HPP

#include <memory>
#include <optional>
#include <thread>
#include <vector>

#include <vulkan/vulkan.hpp>

class Log;
class VarCollection;
class CommandManager;
class GpuAllocator;
class Renderer;
class Swapchain;
class RenderGraphExecutor;
class CommandBufferProxy;
class LogicalDeviceProxy;

class RenderThread {
private:
    struct FrameSync {
        vk::Fence fence;
        vk::Semaphore imageAvailableSemaphore;
        vk::Semaphore renderFinishedSemaphore;
    };

    Renderer *_renderer;
    std::shared_ptr<Log> _log;
    std::shared_ptr<VarCollection> _varCollection;
    std::shared_ptr<CommandManager> _commandManager;
    std::shared_ptr<GpuAllocator> _gpuAllocator;
    std::shared_ptr<Swapchain> _swapchain;
    std::shared_ptr<LogicalDeviceProxy> _logicalDevice;

    std::optional<std::shared_ptr<RenderGraphExecutor>> _renderGraphExecutor;

    uint32_t _inflightFrameCount;
    uint32_t _currentFrameIdx;
    std::vector<FrameSync> _frameSyncs;
    std::vector<std::shared_ptr<CommandBufferProxy>> _commandBuffers;

    std::jthread _thread;

    void render();
    void threadFunc(const std::stop_token &stopToken);

    void handleSwapchainInvalidation();
    void handleRenderGraphInvalidation();

public:
    RenderThread(Renderer *renderer,
                 const std::shared_ptr<Log> &log,
                 const std::shared_ptr<VarCollection> &varCollection,
                 const std::shared_ptr<CommandManager> &commandManager,
                 const std::shared_ptr<GpuAllocator> &gpuAllocator,
                 const std::shared_ptr<Swapchain> &swapchain,
                 const std::shared_ptr<LogicalDeviceProxy> &logicalDevice);

    void run();
    void stop();
};

#endif // RENDERING_RENDERTHREAD_HPP
