#include "Renderer.hpp"

#include <limits>
#include <string_view>

#include "src/Engine/EngineError.hpp"
#include "src/Engine/Log.hpp"
#include "src/Rendering/CommandManager.hpp"
#include "src/Rendering/GpuManager.hpp"
#include "src/Rendering/Swapchain.hpp"
#include "src/Rendering/SwapchainManager.hpp"
#include "src/Rendering/Proxies/CommandBufferProxy.hpp"
#include "src/Rendering/Proxies/LogicalDeviceProxy.hpp"

static constexpr const std::string_view RENDERER_TAG = "Renderer";

static constexpr const uint64_t RENDERER_WAIT_TIMEOUT = 16 * 1000;

void Renderer::render() {
    FrameSync frameSync = this->_frameSyncs[this->_currentFrameIdx];

    if (this->_logicalDevice->getHandle().waitForFences(frameSync.fence, true, RENDERER_WAIT_TIMEOUT) !=
        vk::Result::eSuccess) {
        throw EngineError("Frame fence timeout");
    }

    this->_logicalDevice->getHandle().resetFences(frameSync.fence);

    auto [result, imageIdx] = this->_logicalDevice->getHandle().acquireNextImageKHR(this->_swapchain->getHandle(),
                                                                                    RENDERER_WAIT_TIMEOUT,
                                                                                    frameSync.imageAvailableSemaphore);

    if (result != vk::Result::eSuccess) {
        return;
    }

    auto commandBuffer = this->_commandBuffers[this->_currentFrameIdx]->getHandle();

    commandBuffer.begin(vk::CommandBufferBeginInfo());
    // TODO: render
    commandBuffer.end();

    auto waitDstStageMask = {static_cast<vk::PipelineStageFlags>(vk::PipelineStageFlagBits::eColorAttachmentOutput)};

    auto submitInfo = vk::SubmitInfo()
            .setCommandBuffers(commandBuffer)
            .setWaitSemaphores(frameSync.imageAvailableSemaphore)
            .setSignalSemaphores(frameSync.renderFinishedSemaphore)
            .setWaitDstStageMask(waitDstStageMask);

    this->_logicalDevice->getGraphicsQueue().submit(submitInfo);

    auto presentInfo = vk::PresentInfoKHR()
            .setWaitSemaphores(frameSync.renderFinishedSemaphore)
            .setSwapchains(this->_swapchain->getHandle())
            .setImageIndices(imageIdx);

    if (this->_logicalDevice->getPresentQueue().presentKHR(presentInfo) != vk::Result::eSuccess) {
        // TODO: resize happened
    }

    this->_currentFrameIdx = (this->_currentFrameIdx + 1) % this->_inflightFrameCount;
}

Renderer::Renderer(const std::shared_ptr<Log> &log,
                   const std::shared_ptr<GpuManager> &gpuManager,
                   const std::shared_ptr<Window> &window)
        : _log(log),
          _gpuManager(gpuManager),
          _window(window) {
    //
}

void Renderer::init() {
    if (this->_gpuManager->getLogicalDeviceProxy().expired() ||
        this->_gpuManager->getCommandManager().expired() ||
        this->_gpuManager->getSwapchainManager().expired()) {
        throw EngineError("Failed to initialize renderer - GPU manager is not initialized");
    }

    this->_logicalDevice = this->_gpuManager->getLogicalDeviceProxy().lock();
    auto commandManager = this->_gpuManager->getCommandManager().lock();
    auto swapchainManager = this->_gpuManager->getSwapchainManager().lock();

    // TODO: handle exceptions

    this->_inflightFrameCount = 2; // TODO: extract from variables

    this->_frameSyncs = std::vector<FrameSync>(this->_inflightFrameCount);
    this->_commandBuffers = std::vector<std::shared_ptr<CommandBufferProxy>>(this->_inflightFrameCount);

    auto fenceCreateInfo = vk::FenceCreateInfo()
            .setFlags(vk::FenceCreateFlagBits::eSignaled);
    auto semaphoreCreateInfo = vk::SemaphoreCreateInfo();

    for (uint32_t frameIdx = 0; frameIdx < this->_inflightFrameCount; frameIdx++) {
        this->_frameSyncs[frameIdx] = {
                .fence = this->_logicalDevice->getHandle().createFence(fenceCreateInfo),
                .imageAvailableSemaphore = this->_logicalDevice->getHandle().createSemaphore(semaphoreCreateInfo),
                .renderFinishedSemaphore = this->_logicalDevice->getHandle().createSemaphore(semaphoreCreateInfo)
        };

        this->_commandBuffers[frameIdx] = commandManager->createPrimaryBuffer();
    }

    this->_swapchain = swapchainManager->getSwapchainFor(this->_window);
    this->_swapchain->create();

    this->_renderThread = std::jthread([this](std::stop_token stopToken) {
        while (!stopToken.stop_requested()) {
            try {
                this->render();
            } catch (const std::exception &error) {
                this->_log->error(RENDERER_TAG, error);
            }
        }
    });
}

void Renderer::destroy() {
    this->_renderThread.request_stop();
    this->_renderThread.join();

    this->_swapchain->destroy();

    for (const auto &commandBuffer: this->_commandBuffers) {
        commandBuffer->destroy();
    }

    this->_commandBuffers.clear();

    for (const auto &frameSync: this->_frameSyncs) {
        this->_logicalDevice->getHandle().destroy(frameSync.fence);
        this->_logicalDevice->getHandle().destroy(frameSync.imageAvailableSemaphore);
        this->_logicalDevice->getHandle().destroy(frameSync.renderFinishedSemaphore);
    }

    this->_frameSyncs.clear();

    this->_logicalDevice = nullptr;
}
