#ifndef RENDERING_RENDERER_HPP
#define RENDERING_RENDERER_HPP

#include <memory>
#include <thread>
#include <vector>

#include <vulkan/vulkan.hpp>

#include "src/Rendering/Stages/RenderStage.hpp"

class Log;
class GpuManager;
class Swapchain;
class CommandBufferProxy;
class LogicalDeviceProxy;
class Window;

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

    std::vector<std::unique_ptr<RenderStage>> _stages;

    void render();

public:
    Renderer(const std::shared_ptr<Log> &log,
             const std::shared_ptr<GpuManager> &gpuManager,
             const std::shared_ptr<Window> &window);

    void init();
    void destroy();

    void addRenderStage(std::unique_ptr<RenderStage> &&stage);
};

#endif // RENDERING_RENDERER_HPP
