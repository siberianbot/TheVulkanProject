#include "Renderer.hpp"

#include <limits>
#include <string_view>

#include "src/Engine/EngineError.hpp"
#include "src/Engine/Log.hpp"
#include "src/Engine/VarCollection.hpp"
#include "src/Engine/Vars.hpp"
#include "src/Rendering/CommandManager.hpp"
#include "src/Rendering/GpuManager.hpp"
#include "src/Rendering/Swapchain.hpp"
#include "src/Rendering/SwapchainManager.hpp"
#include "src/Rendering/Proxies/CommandBufferProxy.hpp"
#include "src/Rendering/Proxies/LogicalDeviceProxy.hpp"

static constexpr const std::string_view RENDERER_TAG = "Renderer";

void Renderer::render() {
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

    for (const auto &stage: this->_stages) {
        if (!stage->isInitialized()) {
            continue;
        }

        stage->draw(imageIdx.value(), commandBuffer->getHandle());
    }

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

Renderer::Renderer(const std::shared_ptr<Log> &log,
                   const std::shared_ptr<VarCollection> &varCollection,
                   const std::shared_ptr<GpuManager> &gpuManager,
                   const std::shared_ptr<Window> &window)
        : _log(log),
          _varCollection(varCollection),
          _gpuManager(gpuManager),
          _window(window) {
    //
}

void Renderer::init() {
    if (this->_gpuManager->getLogicalDeviceProxy().expired() ||
        this->_gpuManager->getCommandManager().expired() ||
        this->_gpuManager->getSwapchainManager().expired()) {
        throw EngineError("GPU manager is not initialized");
    }

    this->_logicalDevice = this->_gpuManager->getLogicalDeviceProxy().lock();
    auto commandManager = this->_gpuManager->getCommandManager().lock();
    auto swapchainManager = this->_gpuManager->getSwapchainManager().lock();

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

        this->_commandBuffers[frameIdx] = commandManager->createPrimaryBuffer();
    }

    this->_swapchain = swapchainManager->getSwapchainFor(this->_window);

    this->_renderThread = std::jthread([this](std::stop_token stopToken) {
        auto exception = []() { return EngineError("Rendering thread failure"); };

        while (!stopToken.stop_requested()) {
            if (this->_swapchain->isInvalid()) {
                this->_logicalDevice->getHandle().waitIdle();

                for (const auto &stage: this->_stages) {
                    if (!stage->isInitialized()) {
                        continue;
                    }

                    stage->destroy();
                }

                try {
                    this->_swapchain->create();
                } catch (const std::exception &error) {
                    this->_log->error(RENDERER_TAG, error);
                    throw exception();
                }

                RenderStageInitContext initContext = {
                        .swapchain = this->_swapchain
                };

                for (const auto &stage: this->_stages) {
                    try {
                        stage->init(initContext);
                    } catch (const std::exception &error) {
                        this->_log->error(RENDERER_TAG, error);
                    }
                }
            }

            try {
                this->render();
            } catch (const vk::OutOfDateKHRError &error) {
                this->_swapchain->invalidate();
            } catch (const std::exception &error) {
                this->_log->error(RENDERER_TAG, error);
                throw exception();
            }
        }
    });
}

void Renderer::destroy() {
    this->_renderThread.request_stop();
    this->_renderThread.join();

    this->_logicalDevice->getHandle().waitIdle();

    for (const auto &stage: this->_stages) {
        if (!stage->isInitialized()) {
            continue;
        }

        stage->destroy();
    }

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

void Renderer::addRenderStage(std::unique_ptr<RenderStage> &&stage) {
    this->_stages.push_back(std::move(stage));
}
