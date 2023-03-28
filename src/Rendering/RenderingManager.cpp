#include "RenderingManager.hpp"

#include <iostream>
#include <string>

#include <GLFW/glfw3.h>

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
    std::string type;
    switch (messageType) {
        case VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT:
            type = "Vulkan-Validation";
            break;

        case VK_DEBUG_UTILS_MESSAGE_TYPE_PERFORMANCE_BIT_EXT:
            type = "Vulkan-Performance";
            break;

        case VK_DEBUG_UTILS_MESSAGE_TYPE_DEVICE_ADDRESS_BINDING_BIT_EXT:
            type = "Vulkan-Binding";
            break;

        case VK_DEBUG_UTILS_MESSAGE_TYPE_GENERAL_BIT_EXT:
            type = "Vulkan-General";
            break;

        default:
            throw std::runtime_error("Not supported");
    }

    if (messageSeverity == VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT) {
        std::cerr << type << ": " << pCallbackData->pMessage << std::endl;
    } else {
        std::cout << type << ": " << pCallbackData->pMessage << std::endl;
    }

    return VK_FALSE;
}

VkInstance RenderingManager::createInstance() {
    uint32_t count;
    const char **extensionsPtr = glfwGetRequiredInstanceExtensions(&count);

    std::vector<const char *> extensions(extensionsPtr, extensionsPtr + count);
    extensions.push_back(VK_EXT_DEBUG_UTILS_EXTENSION_NAME);

    std::string name = "TheVulkanProject";
    uint32_t version = VK_MAKE_VERSION(0, 1, 0);

    VkApplicationInfo appInfo = {
            .sType = VK_STRUCTURE_TYPE_APPLICATION_INFO,
            .pNext = nullptr,
            .pApplicationName = name.c_str(),
            .applicationVersion = version,
            .pEngineName = name.c_str(),
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
            .enabledLayerCount = static_cast<uint32_t>(VALIDATION_LAYERS.size()),
            .ppEnabledLayerNames = VALIDATION_LAYERS.data(),
            .enabledExtensionCount = static_cast<uint32_t>(extensions.size()),
            .ppEnabledExtensionNames = extensions.data()
    };

    VkInstance instance;
    vkEnsure(vkCreateInstance(&instanceCreateInfo, nullptr, &instance));

    return instance;
}

VkSurfaceKHR RenderingManager::createSurface() {
    VkSurfaceKHR surface;
    vkEnsure(glfwCreateWindowSurface(this->_instance, this->_window->handle(), nullptr, &surface));

    return surface;
}

RenderingManager::RenderingManager(const std::shared_ptr<VarCollection> &vars,
                                   const std::shared_ptr<Window> &window)
        : _vars(vars),
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
