#include "RenderThread.hpp"

#include <limits>
#include <string_view>

#include "src/Engine/EngineError.hpp"
#include "src/Engine/Log.hpp"
#include "src/Engine/VarCollection.hpp"
#include "src/Engine/Vars.hpp"
#include "src/Rendering/CommandManager.hpp"
#include "src/Rendering/Renderer.hpp"
#include "src/Rendering/Swapchain.hpp"
#include "src/Rendering/Graph/RenderGraphExecutor.hpp"
#include "src/Rendering/Proxies/CommandBufferProxy.hpp"
#include "src/Rendering/Proxies/LogicalDeviceProxy.hpp"

static constexpr const std::string_view RENDER_THREAD_TAG = "RenderThread";

void RenderThread::render() {
    if (!this->_renderGraphExecutor.has_value()) {
        return;
    }

    auto frameSync = this->_frameSyncs[this->_currentFrameIdx];

    if (this->_logicalDevice->getHandle().waitForFences(frameSync.fence, true, std::numeric_limits<uint64_t>::max()) ==
        vk::Result::eTimeout) {
        throw EngineError("Frame fence timeout");
    }

    this->_logicalDevice->getHandle().resetFences(frameSync.fence);

    auto imageIdx = this->_swapchain->acquireNextImage(frameSync.imageAvailableSemaphore);

    if (!imageIdx.has_value()) {
        this->_swapchain->invalidate();
        return;
    }

    auto commandBuffer = this->_commandBuffers[this->_currentFrameIdx];

    commandBuffer->reset();
    commandBuffer->getHandle().begin(vk::CommandBufferBeginInfo());

    this->_renderGraphExecutor.value()->execute(imageIdx.value(), commandBuffer->getHandle());

    commandBuffer->getHandle().end();

    auto waitDstStageMask = {
            static_cast<vk::PipelineStageFlags>(vk::PipelineStageFlagBits::eColorAttachmentOutput)
    };

    auto submitInfo = vk::SubmitInfo()
            .setCommandBuffers(commandBuffer->getHandle())
            .setWaitSemaphores(frameSync.imageAvailableSemaphore)
            .setSignalSemaphores(frameSync.renderFinishedSemaphore)
            .setWaitDstStageMask(waitDstStageMask);

    this->_logicalDevice->getGraphicsQueue().submit(submitInfo, frameSync.fence);

    auto presentInfo = vk::PresentInfoKHR()
            .setWaitSemaphores(frameSync.renderFinishedSemaphore)
            .setSwapchains(this->_swapchain->getHandle())
            .setImageIndices(imageIdx.value());

    if (this->_logicalDevice->getPresentQueue().presentKHR(presentInfo) != vk::Result::eSuccess) {
        this->_swapchain->invalidate();
    }

    this->_currentFrameIdx = (this->_currentFrameIdx + 1) % this->_inflightFrameCount;
}

void RenderThread::threadFunc(const std::stop_token &stopToken) {
    auto exception = []() { return EngineError("Render thread failure"); };

    while (!stopToken.stop_requested()) {
        this->handleSwapchainInvalidation();
        this->handleRenderGraphInvalidation();

        try {
            this->render();
        } catch (const vk::OutOfDateKHRError &error) {
            this->_swapchain->invalidate();
        } catch (const std::exception &error) {
            this->_log->error(RENDER_THREAD_TAG, error);
            throw exception();
        }
    }
}

void RenderThread::handleSwapchainInvalidation() {
    auto exception = []() { return EngineError("Failed to handle swapchain invalidation"); };

    if (!this->_swapchain->isInvalid()) {
        return;
    }

    this->_logicalDevice->getHandle().waitIdle();

    try {
        this->_swapchain->create();
    } catch (const std::exception &error) {
        this->_log->error(RENDER_THREAD_TAG, error);
        throw exception();
    }

    if (this->_renderGraphExecutor.has_value()) {
        try {
            this->_renderGraphExecutor.value()->recreateFrameBuffers();
        } catch (const std::exception &error) {
            this->_log->error(RENDER_THREAD_TAG, error);
            throw exception();
        }
    }
}

void RenderThread::handleRenderGraphInvalidation() {
    auto exception = []() { return EngineError("Failed to handle render graph invalidation"); };

    if (!this->_renderer->getRenderGraph().has_value()) {
        if (this->_renderGraphExecutor.has_value()) {
            this->_renderGraphExecutor.value()->destroy();
            this->_renderGraphExecutor = std::nullopt;
        }

        return;
    }

    if (this->_renderGraphExecutor.has_value()) {
        if (this->_renderGraphExecutor.value()->getGraph() == this->_renderer->getRenderGraph().value()) {
            return;
        } else {
            this->_renderGraphExecutor.value()->destroy();
        }
    }

    this->_renderGraphExecutor = std::make_shared<RenderGraphExecutor>(this->_renderer,
                                                                       this->_gpuAllocator,
                                                                       this->_swapchain,
                                                                       this->_logicalDevice,
                                                                       this->_renderer->getRenderGraph().value());

    try {
        this->_renderGraphExecutor.value()->create();
    } catch (const std::exception &error) {
        this->_log->error(RENDER_THREAD_TAG, error);
        throw exception();
    }
}

RenderThread::RenderThread(Renderer *renderer,
                           const std::shared_ptr<Log> &log,
                           const std::shared_ptr<VarCollection> &varCollection,
                           const std::shared_ptr<CommandManager> &commandManager,
                           const std::shared_ptr<GpuAllocator> &gpuAllocator,
                           const std::shared_ptr<Swapchain> &swapchain,
                           const std::shared_ptr<LogicalDeviceProxy> &logicalDevice)
        : _renderer(renderer),
          _log(log),
          _varCollection(varCollection),
          _commandManager(commandManager),
          _gpuAllocator(gpuAllocator),
          _swapchain(swapchain),
          _logicalDevice(logicalDevice) {
    //
}

void RenderThread::run() {
    this->_inflightFrameCount = this->_varCollection->getIntOrDefault(RENDERING_INFLIGHT_FRAME_COUNT, 2);

    this->_frameSyncs = std::vector<FrameSync>(this->_inflightFrameCount);
    this->_commandBuffers = std::vector<std::shared_ptr<CommandBufferProxy >>(this->_inflightFrameCount);

    auto fenceCreateInfo = vk::FenceCreateInfo()
            .setFlags(vk::FenceCreateFlagBits::eSignaled);
    auto semaphoreCreateInfo = vk::SemaphoreCreateInfo();

    for (uint32_t frameIdx = 0; frameIdx < this->_inflightFrameCount; frameIdx++) {
        this->_frameSyncs[frameIdx] = {
                .fence = this->_logicalDevice->getHandle().createFence(fenceCreateInfo),
                .imageAvailableSemaphore = this->_logicalDevice->getHandle().createSemaphore(semaphoreCreateInfo),
                .renderFinishedSemaphore = this->_logicalDevice->getHandle().createSemaphore(semaphoreCreateInfo)
        };

        this->_commandBuffers[frameIdx] = this->_commandManager->createPrimaryBuffer();
    }

    this->_thread = std::jthread([this](std::stop_token stopToken) {
        this->threadFunc(stopToken);
    });
}

void RenderThread::stop() {
    if (this->_thread.request_stop()) {
        this->_thread.join();
    }

    this->_logicalDevice->getHandle().waitIdle();

    if (this->_renderGraphExecutor.has_value()) {
        this->_renderGraphExecutor.value()->destroy();
    }

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
}
