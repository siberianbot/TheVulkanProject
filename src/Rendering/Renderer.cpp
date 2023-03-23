#include "Renderer.hpp"

#include <iostream>

#include "src/Constants.hpp"
#include "src/Engine/Engine.hpp"
#include "src/Events/EventQueue.hpp"
#include "CommandExecution.hpp"
#include "CommandExecutor.hpp"
#include "PhysicalDevice.hpp"
#include "RenderingDevice.hpp"
#include "RenderingObjectsAllocator.hpp"
#include "Swapchain.hpp"
#include "src/Rendering/Objects/FenceObject.hpp"
#include "src/Rendering/Objects/SemaphoreObject.hpp"
#include "src/Rendering/Renderpasses/ImguiRenderpass.hpp"
//#include "src/Rendering/Renderpasses/SceneRenderpass.hpp"
#include "src/Rendering/Renderpasses/ShadowRenderpass.hpp"
#include "src/Rendering/Renderpasses/SwapchainPresentRenderpass.hpp"
#include "src/System/Window.hpp"
#include "src/Rendering/VulkanObjectsAllocator.hpp"

VKAPI_ATTR VkBool32 VKAPI_CALL Renderer::debugCallback(VkDebugUtilsMessageSeverityFlagBitsEXT messageSeverity,
                                                       VkDebugUtilsMessageTypeFlagsEXT messageType,
                                                       const VkDebugUtilsMessengerCallbackDataEXT *pCallbackData,
                                                       void *pUserData) {

}

Renderer::Renderer(Engine *engine) : _engine(engine) {
    //
}

void Renderer::init() {
    this->_instance = createInstance();
    this->_surface = createSurface(this->_engine->window()->handle());

    this->_physicalDevice = PhysicalDevice::selectSuitable(this->_instance, this->_surface);
    this->_renderingDevice = RenderingDevice::fromPhysicalDevice(this->_physicalDevice);
    this->_vulkanObjectsAllocator = std::make_shared<VulkanObjectsAllocator>(this->_physicalDevice,
                                                                             this->_renderingDevice);
    this->_commandExecutor = std::make_shared<CommandExecutor>(this->_renderingDevice,
                                                               this->_vulkanObjectsAllocator);
    this->_commandExecutor->init();
    this->_swapchain = std::make_shared<Swapchain>(this->_physicalDevice, this->_renderingDevice,
                                                   this->_vulkanObjectsAllocator);
    this->_renderingObjectsAllocator = std::make_shared<RenderingObjectsAllocator>(this->_renderingDevice,
                                                                           this->_vulkanObjectsAllocator,
                                                                           this->_commandExecutor);

    for (uint32_t frameIdx = 0; frameIdx < MAX_INFLIGHT_FRAMES; frameIdx++) {
        this->_syncObjectsGroups[frameIdx] = new SyncObjectsGroup{
                .fence =  FenceObject::create(this->_renderingDevice, this->_vulkanObjectsAllocator, true),
                .imageAvailableSemaphore = SemaphoreObject::create(this->_vulkanObjectsAllocator),
                .renderFinishedSemaphore = SemaphoreObject::create(this->_vulkanObjectsAllocator)
        };
    }

    this->_swapchain->create();

    RenderpassBase *shadowRenderpass = new ShadowRenderpass(this->_renderingDevice, this->_physicalDevice,
                                                            this->_vulkanObjectsAllocator, this->_engine);
//    SceneRenderpass *sceneRenderpass = new SceneRenderpass(this->_renderingDevice, this->_physicalDevice,
//                                                           this->_vulkanObjectsAllocator, this->_swapchain.get(),
//                                                           this->_engine);
//    sceneRenderpass->addShadowRenderpass(shadowRenderpass);

    RenderpassBase *imguiRenderpass = new ImguiRenderpass(this->_renderingDevice, this->_vulkanObjectsAllocator,
                                                          this->_swapchain.get(), this->_instance,
                                                          this->_physicalDevice.get(), this->_commandExecutor.get());

    SwapchainPresentRenderpass *presentRenderpass = new SwapchainPresentRenderpass(this->_renderingDevice,
                                                                                   this->_swapchain.get(),
                                                                                   this->_engine);
//    presentRenderpass->addInputRenderpass(sceneRenderpass);
    presentRenderpass->addInputRenderpass(imguiRenderpass);

    this->_renderpasses.push_back(shadowRenderpass);
//    this->_renderpasses.push_back(sceneRenderpass);
    this->_renderpasses.push_back(imguiRenderpass);
    this->_renderpasses.push_back(presentRenderpass);

    this->_engine->eventQueue()->addHandler([this](const Event &event) {
        switch (event.type) {
            case RENDERER_RELOADING_REQUESTED_EVENT:
                this->cleanupRenderpasses();
                this->initRenderpasses();
                break;

            case RESIZE_WINDOW_EVENT:
                this->handleResize();
                break;

            default:
                break;
        }
    });
}

void Renderer::cleanup() {
    this->cleanupRenderpasses();

    this->_swapchain->destroy();

    for (RenderpassBase *renderpass: this->_renderpasses) {
        delete renderpass;
    }

    for (uint32_t frameIdx = 0; frameIdx < MAX_INFLIGHT_FRAMES; frameIdx++) {
        this->_syncObjectsGroups[frameIdx]->fence->destroy();
        this->_syncObjectsGroups[frameIdx]->imageAvailableSemaphore->destroy();
        this->_syncObjectsGroups[frameIdx]->renderFinishedSemaphore->destroy();
        delete this->_syncObjectsGroups[frameIdx];
    }

    this->_commandExecutor->destroy();
    this->_renderingDevice->destroy();

    vkDestroySurfaceKHR(this->_instance, this->_surface, nullptr);
    vkDestroyInstance(this->_instance, nullptr);
}

void Renderer::initRenderpasses() {
    for (RenderpassBase *renderpass: this->_renderpasses) {
        renderpass->initRenderpass();
        renderpass->createFramebuffers();
    }
}

void Renderer::cleanupRenderpasses() {
    this->_renderingDevice->waitIdle();

    for (RenderpassBase *renderpass: this->_renderpasses) {
        renderpass->destroyFramebuffers();
        renderpass->destroyRenderpass();
    }
}

VkInstance Renderer::createInstance() {
}

VkSurfaceKHR Renderer::createSurface(GLFWwindow *window) {
}

void Renderer::handleResize() {
    this->_renderingDevice->waitIdle();

    for (RenderpassBase *renderpass: this->_renderpasses) {
        renderpass->destroyFramebuffers();
    }

    this->_swapchain->create();

    for (RenderpassBase *renderpass: this->_renderpasses) {
        renderpass->createFramebuffers();
    }
}

void Renderer::render() {
    SyncObjectsGroup *currentSyncObjects = this->_syncObjectsGroups[_currentFrameIdx];

    currentSyncObjects->fence->wait(UINT64_MAX);
    currentSyncObjects->fence->reset();

    std::optional<uint32_t> acquireResult = this->_renderingDevice
            ->acquireNextSwapchainImageIdx(this->_swapchain->getHandle(), UINT64_MAX,
                                           currentSyncObjects->imageAvailableSemaphore->getHandle());

    if (!acquireResult.has_value()) {
        handleResize();
        return;
    }

    uint32_t imageIdx = acquireResult.value();

    this->_commandExecutor->beginMainExecution(_currentFrameIdx, [this, &imageIdx](VkCommandBuffer cmdBuffer) {
                VkExtent2D extent = this->_swapchain->getSwapchainExtent();

                VkRect2D renderArea = {
                        .offset = {0, 0},
                        .extent = extent
                };

                for (RenderpassBase *renderpass: this->_renderpasses) {
                    renderpass->recordCommands(cmdBuffer, renderArea, _currentFrameIdx, imageIdx);
                }
            })
            .withFence(currentSyncObjects->fence)
            .withWaitSemaphore(currentSyncObjects->imageAvailableSemaphore)
            .withSignalSemaphore(currentSyncObjects->renderFinishedSemaphore)
            .withWaitDstStageMask(VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT)
            .submit(false);

    VkSwapchainKHR swapchain = this->_swapchain->getHandle();
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

    VkResult presentResult = vkQueuePresentKHR(this->_renderingDevice->getPresentQueue(), &presentInfo);

    if (presentResult == VK_ERROR_OUT_OF_DATE_KHR || presentResult == VK_SUBOPTIMAL_KHR) {
        handleResize();
    } else if (presentResult != VkResult::VK_SUCCESS) {
        throw std::runtime_error("Vulkan runtime error");
    }

    _currentFrameIdx = (_currentFrameIdx + 1) % MAX_INFLIGHT_FRAMES;
}

void Renderer::wait() {
    this->_renderingDevice->waitIdle();
}
