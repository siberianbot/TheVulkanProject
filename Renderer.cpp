#include "Renderer.hpp"

#include <set>
#include <iostream>
#include <array>

#include "Constants.hpp"
#include "Engine.hpp"
#include "Rendering/Renderpasses/ClearRenderpass.hpp"
#include "Rendering/Renderpasses/FinalRenderpass.hpp"
#include "VulkanCommon.hpp"

std::vector<const char *> vkRequiredExtensions() {
    uint32_t count;
    const char **extensions = glfwGetRequiredInstanceExtensions(&count);

    std::vector<const char *> result(extensions, extensions + count);
    result.push_back(VK_EXT_DEBUG_UTILS_EXTENSION_NAME);

    return result;
}

static VKAPI_ATTR VkBool32 VKAPI_CALL vkDebugCallback(VkDebugUtilsMessageSeverityFlagBitsEXT messageSeverity,
                                                      VkDebugUtilsMessageTypeFlagsEXT messageType,
                                                      const VkDebugUtilsMessengerCallbackDataEXT *pCallbackData,
                                                      void *pUserData) {
    const char *type;
    switch (messageType) {
        case VkDebugUtilsMessageTypeFlagBitsEXT::VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT:
            type = "vk validation";
            break;

        case VkDebugUtilsMessageTypeFlagBitsEXT::VK_DEBUG_UTILS_MESSAGE_TYPE_PERFORMANCE_BIT_EXT:
            type = "vk performance";
            break;

        case VkDebugUtilsMessageTypeFlagBitsEXT::VK_DEBUG_UTILS_MESSAGE_TYPE_DEVICE_ADDRESS_BINDING_BIT_EXT:
            type = "vk binding";
            break;

        case VkDebugUtilsMessageTypeFlagBitsEXT::VK_DEBUG_UTILS_MESSAGE_TYPE_GENERAL_BIT_EXT:
            type = "vk general";
            break;

        default:
            throw std::runtime_error("not supported");
    }

    if (messageSeverity == VkDebugUtilsMessageSeverityFlagBitsEXT::VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT) {
        std::cerr << type << ": " << pCallbackData->pMessage << std::endl;
    } else {
        std::cout << type << ": " << pCallbackData->pMessage << std::endl;
    }

    return VK_FALSE;
}

Renderer::Renderer(Engine *engine) : engine(engine) {
    //
}

void Renderer::init() {
    initInstance();
    initSurface(this->engine->window());

    this->_physicalDevice = VulkanPhysicalDevice::selectSuitable(this->instance, this->surface);
    this->_renderingDevice = this->_physicalDevice->createRenderingDevice();
    this->_renderingObjectsFactory = new RenderingObjectsFactory(this->_renderingDevice);
    this->_commandExecutor = new VulkanCommandExecutor(this->_renderingDevice);
    this->_swapchain = new Swapchain(this->_renderingDevice, this->_renderingObjectsFactory);

    for (uint32_t frameIdx = 0; frameIdx < MAX_INFLIGHT_FRAMES; frameIdx++) {
        this->_syncObjectsGroups[frameIdx] = new SyncObjectsGroup{
                .fence =  this->_renderingObjectsFactory->createFenceObject(true),
                .imageAvailableSemaphore = this->_renderingObjectsFactory->createSemaphoreObject(),
                .renderFinishedSemaphore = this->_renderingObjectsFactory->createSemaphoreObject()
        };
    }

    this->_swapchain->create();

    this->_renderpasses.push_back(new ClearRenderpass(this->_renderingDevice, this->_swapchain));
    this->_sceneRenderpass = new SceneRenderpass(this->_renderingDevice, this->_swapchain,
                                                 this->_renderingObjectsFactory,
                                                 this->engine, this->_commandExecutor);
    this->_renderpasses.push_back(this->_sceneRenderpass);
    this->_renderpasses.push_back(new FinalRenderpass(this->_renderingDevice, this->_swapchain));

    for (RenderpassBase *renderpass: this->_renderpasses) {
        renderpass->initRenderpass();
        renderpass->createFramebuffers();
    }
}

void Renderer::cleanup() {
    this->_renderingDevice->waitIdle();

    for (RenderpassBase *renderpass: this->_renderpasses) {
        renderpass->destroyFramebuffers();
    }

    this->_swapchain->destroy();

    this->_sceneRenderpass = nullptr;
    for (RenderpassBase *renderpass: this->_renderpasses) {
        renderpass->destroyRenderpass();
        delete renderpass;
    }

    for (uint32_t frameIdx = 0; frameIdx < MAX_INFLIGHT_FRAMES; frameIdx++) {
        delete this->_syncObjectsGroups[frameIdx];
    }

    delete this->_swapchain;
    delete this->_commandExecutor;
    delete this->_renderingObjectsFactory;
    delete this->_renderingDevice;
    delete this->_physicalDevice;

    vkDestroySurfaceKHR(this->instance, this->surface, nullptr);
    vkDestroyInstance(this->instance, nullptr);
}

void Renderer::requestResize(uint32_t width, uint32_t height) {
    this->resizeRequested = true;
}

void Renderer::initInstance() {
    auto extensions = vkRequiredExtensions();
    uint32_t version = VK_MAKE_VERSION(0, 1, 0);

    VkApplicationInfo appInfo = {
            .sType = VkStructureType::VK_STRUCTURE_TYPE_APPLICATION_INFO,
            .pNext = nullptr,
            .pApplicationName = NAME,
            .applicationVersion = version,
            .pEngineName = NAME,
            .engineVersion = version,
            .apiVersion = VK_API_VERSION_1_1,
    };

    VkDebugUtilsMessengerCreateInfoEXT debugUtilsMessengerCreateInfoExt = {
            .sType = VkStructureType::VK_STRUCTURE_TYPE_DEBUG_UTILS_MESSENGER_CREATE_INFO_EXT,
            .pNext = nullptr,
            .flags = 0,
            .messageSeverity = VkDebugUtilsMessageSeverityFlagBitsEXT::VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT |
                               VkDebugUtilsMessageSeverityFlagBitsEXT::VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT |
                               VkDebugUtilsMessageSeverityFlagBitsEXT::VK_DEBUG_UTILS_MESSAGE_SEVERITY_INFO_BIT_EXT |
                               VkDebugUtilsMessageSeverityFlagBitsEXT::VK_DEBUG_UTILS_MESSAGE_SEVERITY_VERBOSE_BIT_EXT,
            .messageType = VkDebugUtilsMessageTypeFlagBitsEXT::VK_DEBUG_UTILS_MESSAGE_TYPE_GENERAL_BIT_EXT |
                           VkDebugUtilsMessageTypeFlagBitsEXT::VK_DEBUG_UTILS_MESSAGE_TYPE_DEVICE_ADDRESS_BINDING_BIT_EXT |
                           VkDebugUtilsMessageTypeFlagBitsEXT::VK_DEBUG_UTILS_MESSAGE_TYPE_PERFORMANCE_BIT_EXT |
                           VkDebugUtilsMessageTypeFlagBitsEXT::VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT,
            .pfnUserCallback = vkDebugCallback,
            .pUserData = this
    };

    VkInstanceCreateInfo instanceCreateInfo = {
            .sType = VkStructureType::VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO,
            .pNext = &debugUtilsMessengerCreateInfoExt,
            .flags = 0,
            .pApplicationInfo = &appInfo,
            .enabledLayerCount = static_cast<uint32_t>(VK_VALIDATION_LAYERS.size()),
            .ppEnabledLayerNames = VK_VALIDATION_LAYERS.data(),
            .enabledExtensionCount = static_cast<uint32_t>(extensions.size()),
            .ppEnabledExtensionNames = extensions.data()
    };

    vkEnsure(vkCreateInstance(&instanceCreateInfo, nullptr, &this->instance));
}

void Renderer::initSurface(GLFWwindow *window) {
    vkEnsure(glfwCreateWindowSurface(this->instance, window, nullptr, &this->surface));
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

    uint32_t imageIdx;
    VkResult result = vkAcquireNextImageKHR(this->_renderingDevice->getHandle(), this->_swapchain->getHandle(),
                                            UINT64_MAX, currentSyncObjects->imageAvailableSemaphore->getHandle(),
                                            VK_NULL_HANDLE, &imageIdx);

    if (result == VK_ERROR_OUT_OF_DATE_KHR) {
        handleResize();
        return;
    } else if (result != VkResult::VK_SUCCESS && result != VkResult::VK_SUBOPTIMAL_KHR) {
        throw std::runtime_error("Vulkan acquire next image failure");
    }

    VkExtent2D extent = this->_swapchain->getSwapchainExtent();

    this->_commandExecutor->beginMainExecution(_currentFrameIdx, [this, &extent, &imageIdx](VkCommandBuffer cmdBuffer) {
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
            .sType = VkStructureType::VK_STRUCTURE_TYPE_PRESENT_INFO_KHR,
            .pNext = nullptr,
            .waitSemaphoreCount = 1,
            .pWaitSemaphores = &renderFinishedSemaphore,
            .swapchainCount = 1,
            .pSwapchains = &swapchain,
            .pImageIndices = &imageIdx,
            .pResults = nullptr
    };

    result = vkQueuePresentKHR(this->_renderingDevice->getPresentQueue(), &presentInfo);

    if (result == VkResult::VK_ERROR_OUT_OF_DATE_KHR || result == VkResult::VK_SUBOPTIMAL_KHR || resizeRequested) {
        resizeRequested = false;
        handleResize();
    } else if (result != VkResult::VK_SUCCESS) {
        throw std::runtime_error("Vulkan present failure");
    }

    _currentFrameIdx = (_currentFrameIdx + 1) % MAX_INFLIGHT_FRAMES;
}
