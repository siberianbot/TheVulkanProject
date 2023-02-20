#include "Renderer.hpp"

#include <iostream>

#include "Constants.hpp"
#include "Engine.hpp"
#include "VulkanCommon.hpp"
#include "Rendering/VulkanPhysicalDevice.hpp"
#include "Rendering/RenderingObjectsFactory.hpp"
#include "Rendering/VulkanCommandExecutor.hpp"
#include "Rendering/Swapchain.hpp"
#include "Rendering/Renderpasses/RenderpassBase.hpp"
#include "Rendering/Renderpasses/ClearRenderpass.hpp"
#include "Rendering/Renderpasses/SceneRenderpass.hpp"
#include "Rendering/Renderpasses/FinalRenderpass.hpp"

Renderer::SyncObjectsGroup::~SyncObjectsGroup() {
    delete this->fence;
    delete this->imageAvailableSemaphore;
    delete this->renderFinishedSemaphore;
}

VKAPI_ATTR VkBool32 VKAPI_CALL Renderer::debugCallback(VkDebugUtilsMessageSeverityFlagBitsEXT messageSeverity,
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

Renderer::Renderer(Engine *engine) : _engine(engine) {
    //
}

void Renderer::init() {
    this->_instance = createInstance();
    this->_surface = createSurface(this->_engine->window());

    this->_physicalDevice = VulkanPhysicalDevice::selectSuitable(this->_instance, this->_surface);
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
                                                 this->_engine, this->_commandExecutor);
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

    vkDestroySurfaceKHR(this->_instance, this->_surface, nullptr);
    vkDestroyInstance(this->_instance, nullptr);
}

void Renderer::requestResize() {
    handleResize();
}

VkInstance Renderer::createInstance() {
    uint32_t count;
    const char **extensionsPtr = glfwGetRequiredInstanceExtensions(&count);

    std::vector<const char *> extensions(extensionsPtr, extensionsPtr + count);
    extensions.push_back(VK_EXT_DEBUG_UTILS_EXTENSION_NAME);

    uint32_t version = VK_MAKE_VERSION(0, 1, 0);

    VkApplicationInfo appInfo = {
            .sType = VK_STRUCTURE_TYPE_APPLICATION_INFO,
            .pNext = nullptr,
            .pApplicationName = NAME,
            .applicationVersion = version,
            .pEngineName = NAME,
            .engineVersion = version,
            .apiVersion = VK_API_VERSION_1_3,
    };

    VkDebugUtilsMessengerCreateInfoEXT debugUtilsMessengerCreateInfoExt = {
            .sType = VK_STRUCTURE_TYPE_DEBUG_UTILS_MESSENGER_CREATE_INFO_EXT,
            .pNext = nullptr,
            .flags = 0,
            .messageSeverity = VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT |
                               VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT |
                               VK_DEBUG_UTILS_MESSAGE_SEVERITY_INFO_BIT_EXT |
                               VK_DEBUG_UTILS_MESSAGE_SEVERITY_VERBOSE_BIT_EXT,
            .messageType = VK_DEBUG_UTILS_MESSAGE_TYPE_GENERAL_BIT_EXT |
                           VK_DEBUG_UTILS_MESSAGE_TYPE_DEVICE_ADDRESS_BINDING_BIT_EXT |
                           VK_DEBUG_UTILS_MESSAGE_TYPE_PERFORMANCE_BIT_EXT |
                           VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT,
            .pfnUserCallback = debugCallback,
            .pUserData = this
    };

    VkInstanceCreateInfo instanceCreateInfo = {
            .sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO,
            .pNext = &debugUtilsMessengerCreateInfoExt,
            .flags = 0,
            .pApplicationInfo = &appInfo,
            .enabledLayerCount = static_cast<uint32_t>(VK_VALIDATION_LAYERS.size()),
            .ppEnabledLayerNames = VK_VALIDATION_LAYERS.data(),
            .enabledExtensionCount = static_cast<uint32_t>(extensions.size()),
            .ppEnabledExtensionNames = extensions.data()
    };

    VkInstance instance;
    vkEnsure(vkCreateInstance(&instanceCreateInfo, nullptr, &instance));

    return instance;
}

VkSurfaceKHR Renderer::createSurface(GLFWwindow *window) {
    VkSurfaceKHR surface;
    vkEnsure(glfwCreateWindowSurface(this->_instance, window, nullptr, &surface));

    return surface;
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
