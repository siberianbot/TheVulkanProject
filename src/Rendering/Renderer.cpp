#include "Renderer.hpp"

#include "src/Events/EventQueue.hpp"
#include "src/Rendering/CommandExecution.hpp"
#include "src/Rendering/CommandExecutor.hpp"
#include "src/Rendering/RenderingDevice.hpp"
#include "src/Rendering/RenderingManager.hpp"
#include "src/Rendering/Swapchain.hpp"
#include "src/Rendering/Objects/FenceObject.hpp"
#include "src/Rendering/Objects/SemaphoreObject.hpp"
#include "src/Rendering/Renderpasses/DebugUIRenderpass.hpp"
#include "src/Rendering/Renderpasses/ShadowRenderpass.hpp"
#include "src/Rendering/Stages/DebugUIStage.hpp"
#include "src/Rendering/Stages/SceneStage.hpp"

void Renderer::handleResize() {
    this->_renderingManager->waitIdle();

    // TODO: reset framebuffers
    this->_renderingManager->swapchain()->create();
}

std::optional<uint32_t> Renderer::acquireNextImageIdx(const std::shared_ptr<SemaphoreObject> &semaphore) {
    uint32_t imageIdx;
    VkResult result = vkAcquireNextImageKHR(this->_renderingManager->renderingDevice()->getHandle(),
                                            this->_renderingManager->swapchain()->getHandle(),
                                            UINT64_MAX, semaphore->getHandle(), VK_NULL_HANDLE, &imageIdx);


    if (result == VK_ERROR_OUT_OF_DATE_KHR) {
        return std::nullopt;
    } else if (result == VK_SUCCESS || result == VK_SUBOPTIMAL_KHR) {
        return imageIdx;
    }

    throw std::runtime_error("Vulkan runtime error");
}

Renderer::Renderer(const std::shared_ptr<EngineVars> &engineVars,
                   const std::shared_ptr<EventQueue> &eventQueue,
                   const std::shared_ptr<RenderingManager> &renderingManager,
                   const std::shared_ptr<ResourceManager> &resourceManager,
                   const std::shared_ptr<SceneManager> &sceneManager)
        : _engineVars(engineVars),
          _eventQueue(eventQueue),
          _renderingManager(renderingManager),
          _resourceManager(resourceManager),
          _sceneManager(sceneManager) {
    //
}

void Renderer::init() {
    this->_eventQueue->addHandler([this](const Event &event) {
        switch (event.type) {
            // TODO: reload stages, renderpasses, framebuffers

            case RESIZE_WINDOW_EVENT:
                this->handleResize();
                break;

            default:
                break;
        }
    });

    for (uint32_t frameIdx = 0; frameIdx < MAX_INFLIGHT_FRAMES; frameIdx++) {
        this->_syncObjectsGroups[frameIdx] = std::make_shared<SyncObjectsGroup>();
        this->_syncObjectsGroups[frameIdx]->fence = FenceObject::create(this->_renderingManager->renderingDevice(),
                                                                        this->_renderingManager->vulkanObjectsAllocator(),
                                                                        true);
        this->_syncObjectsGroups[frameIdx]->imageAvailableSemaphore = SemaphoreObject::create(
                this->_renderingManager->vulkanObjectsAllocator());
        this->_syncObjectsGroups[frameIdx]->renderFinishedSemaphore = SemaphoreObject::create(
                this->_renderingManager->vulkanObjectsAllocator());
    }


    this->_renderingManager->swapchain()->create();

    this->_stages.emplace_back(std::make_shared<SceneStage>(this->_engineVars,
                                                            this->_eventQueue,
                                                            this->_renderingManager,
                                                            this->_resourceManager,
                                                            this->_sceneManager));

    this->_stages.emplace_back(std::make_shared<DebugUIStage>(this->_eventQueue,
                                                              this->_renderingManager));

    for (const auto &stage: this->_stages) {
        stage->init();
    }
}

void Renderer::destroy() {
    this->_renderingManager->waitIdle();

    for (const auto &stage: this->_stages) {
        stage->destroy();
    }

    this->_renderingManager->swapchain()->destroy();

    for (uint32_t frameIdx = 0; frameIdx < MAX_INFLIGHT_FRAMES; frameIdx++) {
        this->_syncObjectsGroups[frameIdx]->fence->destroy();
        this->_syncObjectsGroups[frameIdx]->imageAvailableSemaphore->destroy();
        this->_syncObjectsGroups[frameIdx]->renderFinishedSemaphore->destroy();
        this->_syncObjectsGroups[frameIdx] = nullptr;
    }
}

void Renderer::render() {
    std::shared_ptr<SyncObjectsGroup> currentSyncObjects = this->_syncObjectsGroups[this->_currentFrameIdx];

    currentSyncObjects->fence->wait(UINT64_MAX);
    currentSyncObjects->fence->reset();

    std::optional<uint32_t> acquireResult = this->acquireNextImageIdx(currentSyncObjects->imageAvailableSemaphore);

    if (!acquireResult.has_value()) {
        handleResize();
        return;
    }

    uint32_t imageIdx = acquireResult.value();

    this->_renderingManager->commandExecutor()->beginMainExecution(
                    this->_currentFrameIdx, [&](VkCommandBuffer cmdBuffer) {
                        for (const auto &stage: this->_stages) {
                            stage->record(cmdBuffer, this->_currentFrameIdx, imageIdx);
                        }
                    })
            .withFence(currentSyncObjects->fence)
            .withWaitSemaphore(currentSyncObjects->imageAvailableSemaphore)
            .withSignalSemaphore(currentSyncObjects->renderFinishedSemaphore)
            .withWaitDstStageMask(VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT)
            .submit(false);

    VkSwapchainKHR swapchain = this->_renderingManager->swapchain()->getHandle();
    VkSemaphore renderFinishedSemaphore = currentSyncObjects->renderFinishedSemaphore->getHandle();

    VkPresentInfoKHR presentInfo = {
            .sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR,
            .pNext = nullptr,
            .waitSemaphoreCount = 1,
            .pWaitSemaphores = &renderFinishedSemaphore,
            .swapchainCount = 1,
            .pSwapchains = &swapchain,
            .pImageIndices = &imageIdx,
            .pResults = nullptr
    };

    VkResult presentResult = vkQueuePresentKHR(this->_renderingManager->renderingDevice()->getPresentQueue(),
                                               &presentInfo);

    if (presentResult == VK_ERROR_OUT_OF_DATE_KHR || presentResult == VK_SUBOPTIMAL_KHR) {
        handleResize();
    } else if (presentResult != VkResult::VK_SUCCESS) {
        throw std::runtime_error("Vulkan runtime error");
    }

    this->_currentFrameIdx = (this->_currentFrameIdx + 1) % MAX_INFLIGHT_FRAMES;
}
