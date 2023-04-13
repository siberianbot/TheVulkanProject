#include "RenderingManager.hpp"

#include <string>

#include <GLFW/glfw3.h>

#include "src/Engine/Log.hpp"
#include "src/Engine/VarCollection.hpp"
#include "src/Rendering/Common.hpp"
#include "src/Rendering/CommandExecutor.hpp"
#include "src/Rendering/PhysicalDevice.hpp"
#include "src/Rendering/RenderingDevice.hpp"
#include "src/Rendering/RenderingLayoutsManager.hpp"
#include "src/Rendering/RenderingObjectsAllocator.hpp"
#include "src/Rendering/VulkanObjectsAllocator.hpp"
#include "src/Rendering/Swapchain.hpp"
#include "src/System/Window.hpp"

VkBool32 RenderingManager::debugCallback(VkDebugUtilsMessageSeverityFlagBitsEXT messageSeverity,
                                         VkDebugUtilsMessageTypeFlagsEXT messageType,
                                         const VkDebugUtilsMessengerCallbackDataEXT *pCallbackData,
                                         void *pUserData) {
    RenderingManager *that = reinterpret_cast<RenderingManager *>(pUserData);

    LogCategory category;
    switch (messageSeverity) {
        case VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT:
            category = ERROR_LOG_CATEGORY;
            break;

        case VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT:
            category = WARNING_LOG_CATEGORY;
            break;

        case VK_DEBUG_UTILS_MESSAGE_SEVERITY_INFO_BIT_EXT:
        case VK_DEBUG_UTILS_MESSAGE_SEVERITY_VERBOSE_BIT_EXT:
        default:
            category = INFO_LOG_CATEGORY;
            break;
    }

    std::string tag;
    switch (messageType) {
        case VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT:
            tag = "Vulkan-Validation";
            break;

        case VK_DEBUG_UTILS_MESSAGE_TYPE_PERFORMANCE_BIT_EXT:
            tag = "Vulkan-Performance";
            break;

        case VK_DEBUG_UTILS_MESSAGE_TYPE_DEVICE_ADDRESS_BINDING_BIT_EXT:
            tag = "Vulkan-Binding";
            break;

        case VK_DEBUG_UTILS_MESSAGE_TYPE_GENERAL_BIT_EXT:
            tag = "Vulkan-General";
            break;

        default:
            tag = "Vulkan-N/A";
            break;
    }

    that->_log->push(category, tag, pCallbackData->pMessage);

    return VK_FALSE;
}

VkInstance RenderingManager::createInstance() {
    uint32_t count;
    const char **extensionsPtr = glfwGetRequiredInstanceExtensions(&count);

    std::vector<const char *> extensions(extensionsPtr, extensionsPtr + count);
    extensions.push_back(VK_EXT_DEBUG_UTILS_EXTENSION_NAME);

    std::string name = "TheVulkanProject";
    uint32_t version = VK_MAKE_VERSION(0, 1, 0);

    vk::ApplicationInfo appInfo = vk::ApplicationInfo()
            .setApiVersion(VK_API_VERSION_1_3)
            .setPApplicationName(name.c_str())
            .setApplicationVersion(version)
            .setPEngineName(name.c_str())
            .setEngineVersion(version);

    vk::DebugUtilsMessengerCreateInfoEXT debugMessengerCreateInfo = vk::DebugUtilsMessengerCreateInfoEXT()
            .setPfnUserCallback(debugCallback)
            .setMessageType(vk::DebugUtilsMessageTypeFlagBitsEXT::eGeneral |
                            vk::DebugUtilsMessageTypeFlagBitsEXT::eDeviceAddressBinding |
                            vk::DebugUtilsMessageTypeFlagBitsEXT::ePerformance |
                            vk::DebugUtilsMessageTypeFlagBitsEXT::eValidation)
            .setMessageSeverity(vk::DebugUtilsMessageSeverityFlagBitsEXT::eVerbose |
                                vk::DebugUtilsMessageSeverityFlagBitsEXT::eInfo |
                                vk::DebugUtilsMessageSeverityFlagBitsEXT::eWarning |
                                vk::DebugUtilsMessageSeverityFlagBitsEXT::eError);

    vk::InstanceCreateInfo instanceCreateInfo = vk::InstanceCreateInfo()
            .setPNext(&debugMessengerCreateInfo)
            .setPApplicationInfo(&appInfo)
            .setEnabledLayerCount(VALIDATION_LAYERS.size())
            .setPEnabledLayerNames(VALIDATION_LAYERS)
            .setEnabledExtensionCount(count)
            .setPEnabledExtensionNames(extensions);

    const auto &[isntance, res] = vk::createInstance(instanceCreateInfo);


    VkInstance instance;
    vkEnsure(vkCreateInstance(&instanceCreateInfo, nullptr, &instance));

    return instance;
}

VkSurfaceKHR RenderingManager::createSurface() {
    VkSurfaceKHR surface;
    vkEnsure(glfwCreateWindowSurface(this->_instance, this->_window->handle(), nullptr, &surface));

    return surface;
}

RenderingManager::RenderingManager(const std::shared_ptr<Log> &log,
                                   const std::shared_ptr<VarCollection> &vars,
                                   const std::shared_ptr<Window> &window)
        : _log(log),
          _vars(vars),
          _window(window) {
    //
}

void RenderingManager::init() {
    this->_instance = this->createInstance();
    this->_surface = this->createSurface();

    this->_physicalDevice = PhysicalDevice::selectSuitable(this->_instance, this->_surface);
    this->_renderingDevice = RenderingDevice::fromPhysicalDevice(this->_physicalDevice);

    this->_vulkanObjectsAllocator = std::make_shared<VulkanObjectsAllocator>(this->_physicalDevice,
                                                                             this->_renderingDevice);

    this->_renderingLayoutsManager = std::make_shared<RenderingLayoutsManager>(this->_vars,
                                                                               this->_renderingDevice,
                                                                               this->_vulkanObjectsAllocator);
    this->_renderingLayoutsManager->init();

    this->_commandExecutor = std::make_shared<CommandExecutor>(this->_renderingDevice, this->_vulkanObjectsAllocator);
    this->_commandExecutor->init();

    this->_renderingObjectsAllocator = std::make_shared<RenderingObjectsAllocator>(this->_renderingDevice,
                                                                                   this->_vulkanObjectsAllocator,
                                                                                   this->_commandExecutor);

    this->_swapchain = std::make_shared<Swapchain>(this->_physicalDevice, this->_renderingDevice,
                                                   this->_vulkanObjectsAllocator);
}

void RenderingManager::destroy() {
    this->_swapchain->destroy();
    this->_commandExecutor->destroy();
    this->_renderingLayoutsManager->destroy();
    this->_renderingDevice->destroy();

    vkDestroySurfaceKHR(this->_instance, this->_surface, nullptr);
    vkDestroyInstance(this->_instance, nullptr);
}

void RenderingManager::waitIdle() {
    vkEnsure(vkDeviceWaitIdle(this->_renderingDevice->getHandle()));
}
