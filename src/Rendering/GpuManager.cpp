#include "GpuManager.hpp"

#include <optional>
#include <set>

#include <fmt/core.h>
#include <GLFW/glfw3.h>

#include "src/Constants.hpp"
#include "src/Engine/EngineError.hpp"
#include "src/Engine/Log.hpp"
#include "src/Engine/VarCollection.hpp"
#include "src/Engine/Vars.hpp"
#include "src/Rendering/CommandManager.hpp"
#include "src/Rendering/Extensions.hpp"
#include "src/Rendering/GpuAllocator.hpp"
#include "src/Rendering/Proxies/LogicalDeviceProxy.hpp"
#include "src/Rendering/Proxies/PhysicalDeviceProxy.hpp"
#include "src/System/Window.hpp"

static constexpr const char *GPU_MANAGER_TAG = "GpuManager";

std::vector<const char *> GpuManager::getEnabledExtensions() {
    uint32_t count;
    const char **extensionsPtr = glfwGetRequiredInstanceExtensions(&count);

    std::vector<const char *> extensions(extensionsPtr, extensionsPtr + count);

    for (const char *extension: REQUIRED_INSTANCE_EXTENSIONS) {
        extensions.push_back(extension);
    }

    return extensions;
}

vk::PhysicalDeviceFeatures GpuManager::getEnabledFeatures() {
    vk::PhysicalDeviceFeatures features = vk::PhysicalDeviceFeatures()
            .setSamplerAnisotropy(true);

    return features;
}

void GpuManager::initInstance() {
    std::string name = this->_varCollection->getOrDefault(WINDOW_TITLE_VAR, ENGINE_NAME);
    uint32_t version = VK_MAKE_VERSION(
            ENGINE_VERSION_MAJOR,
            ENGINE_VERSION_MINOR,
            ENGINE_VERSION_PATCH);

    vk::ApplicationInfo appInfo = vk::ApplicationInfo()
            .setApiVersion(VK_API_VERSION_1_3)
            .setPApplicationName(name.c_str())
            .setApplicationVersion(version)
            .setPEngineName(ENGINE_NAME)
            .setEngineVersion(version);

    vk::DebugUtilsMessengerCreateInfoEXT debugMessengerCreateInfo = vk::DebugUtilsMessengerCreateInfoEXT()
            .setPfnUserCallback(messengerCallback)
            .setPUserData(this)
            .setMessageType(vk::DebugUtilsMessageTypeFlagBitsEXT::eGeneral |
                            vk::DebugUtilsMessageTypeFlagBitsEXT::eDeviceAddressBinding |
                            vk::DebugUtilsMessageTypeFlagBitsEXT::ePerformance |
                            vk::DebugUtilsMessageTypeFlagBitsEXT::eValidation)
            .setMessageSeverity(vk::DebugUtilsMessageSeverityFlagBitsEXT::eVerbose |
                                vk::DebugUtilsMessageSeverityFlagBitsEXT::eInfo |
                                vk::DebugUtilsMessageSeverityFlagBitsEXT::eWarning |
                                vk::DebugUtilsMessageSeverityFlagBitsEXT::eError);

    auto enabledExtensions = this->getEnabledExtensions();

    vk::InstanceCreateInfo instanceCreateInfo = vk::InstanceCreateInfo()
            .setPNext(&debugMessengerCreateInfo)
            .setPApplicationInfo(&appInfo)
            .setPEnabledLayerNames(REQUIRED_LAYERS)
            .setPEnabledExtensionNames(enabledExtensions);

    try {
        this->_instance = vk::createInstance(instanceCreateInfo);
    } catch (const std::exception &error) {
        this->_log->error(GPU_MANAGER_TAG, error);

        throw EngineError("Failed to initialize Vulkan instance");
    }
}

void GpuManager::initSurface() {
    VkSurfaceKHR surface = VK_NULL_HANDLE;

    if (glfwCreateWindowSurface(this->_instance, this->_window->handle(), nullptr, &surface) != VK_SUCCESS) {
        throw EngineError("Failed to initialize Vulkan surface");
    }

    this->_surface = vk::SurfaceKHR(surface);
}

void GpuManager::initPhysicalDevice() {
    std::vector<vk::PhysicalDevice> physicalDevices;

    try {
        physicalDevices = this->_instance.enumeratePhysicalDevices();
    } catch (const std::exception &error) {
        this->_log->error(GPU_MANAGER_TAG, error);

        throw EngineError("Failed to initialize Vulkan physical device");
    }

    std::shared_ptr<PhysicalDeviceProxy> selectedPhysicalDevice = nullptr;

    for (const vk::PhysicalDevice &physicalDevice: physicalDevices) {
        vk::PhysicalDeviceProperties properties = physicalDevice.getProperties();

        std::optional<PhysicalDeviceSupportInfo> supportInfo;

        try {
            supportInfo = PhysicalDeviceProxy::getSupportInfoFor(physicalDevice, this->_surface);
        } catch (const std::exception &error) {
            this->_log->error(GPU_MANAGER_TAG, error);

            throw EngineError(fmt::format("Failed to get support info for physical device {0}",
                                          properties.deviceName));
        }

        if (!supportInfo.has_value()) {
            this->_log->info(GPU_MANAGER_TAG, fmt::format("Physical device {0} is not supported, skipping...",
                                                          properties.deviceName));

            continue;
        }

        selectedPhysicalDevice = std::make_shared<PhysicalDeviceProxy>(physicalDevice, properties, supportInfo.value());

        break;
    }

    if (selectedPhysicalDevice == nullptr) {
        throw EngineError("No physical device available");
    }

    this->_physicalDevice = selectedPhysicalDevice;
}

void GpuManager::initLogicalDevice() {
    const std::array<float, 1> queuePriorities = {1.0f};

    std::vector<vk::DeviceQueueCreateInfo> queueCreateInfos;
    std::set<uint32_t> familyIndices = {
            this->_physicalDevice->getGraphicsQueueFamilyIdx(),
            this->_physicalDevice->getPresentQueueFamilyIdx()
    };

    for (uint32_t familyIdx: familyIndices) {
        vk::DeviceQueueCreateInfo queueCreateInfo = vk::DeviceQueueCreateInfo()
                .setQueueFamilyIndex(familyIdx)
                .setQueuePriorities(queuePriorities);

        queueCreateInfos.push_back(queueCreateInfo);
    }

    vk::PhysicalDeviceFeatures features = this->getEnabledFeatures();

    vk::DeviceCreateInfo createInfo = vk::DeviceCreateInfo()
            .setQueueCreateInfos(queueCreateInfos)
            .setPEnabledFeatures(&features)
            .setPEnabledLayerNames(REQUIRED_LAYERS)
            .setPEnabledExtensionNames(REQUIRED_DEVICE_EXTENSIONS);

    vk::Device device = this->_physicalDevice->getHandle().createDevice(createInfo);
    vk::Queue graphicsQueue = device.getQueue(this->_physicalDevice->getGraphicsQueueFamilyIdx(), 0);
    vk::Queue presentQueue = device.getQueue(this->_physicalDevice->getPresentQueueFamilyIdx(), 0);

    this->_logicalDevice = std::make_shared<LogicalDeviceProxy>(device, graphicsQueue, presentQueue);
}

void GpuManager::initCommandManager() {
    this->_commandManager = std::make_shared<CommandManager>(this->_log,
                                                             this->_physicalDevice,
                                                             this->_logicalDevice);

    this->_commandManager->init();
}

void GpuManager::initAllocator() {
    this->_allocator = std::make_shared<GpuAllocator>(this->_log,
                                                      this->_physicalDevice,
                                                      this->_logicalDevice);
}

VkBool32 GpuManager::messengerCallback(VkDebugUtilsMessageSeverityFlagBitsEXT messageSeverity,
                                       VkDebugUtilsMessageTypeFlagsEXT messageTypes,
                                       const VkDebugUtilsMessengerCallbackDataEXT *pCallbackData,
                                       void *pUserData) {
    GpuManager *that = reinterpret_cast<GpuManager *>(pUserData);

    LogCategory category;
    switch (messageSeverity) {
        case VK_DEBUG_UTILS_MESSAGE_SEVERITY_VERBOSE_BIT_EXT:
            category = VERBOSE_LOG_CATEGORY;
            break;

        case VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT:
            category = WARNING_LOG_CATEGORY;
            break;

        case VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT:
            category = ERROR_LOG_CATEGORY;
            break;

        default:
            category = INFO_LOG_CATEGORY;
            break;
    }

    that->_log->push(category, GPU_MANAGER_TAG, pCallbackData->pMessage);

    return 0;
}

GpuManager::GpuManager(const std::shared_ptr<Log> &log,
                       const std::shared_ptr<VarCollection> &varCollection,
                       const std::shared_ptr<Window> &window)
        : _log(log),
          _varCollection(varCollection),
          _window(window) {
    //
}

void GpuManager::init() {
    this->_log->info(GPU_MANAGER_TAG, "Initializing GPU manager...");

    this->initInstance();
    this->initSurface();
    this->initPhysicalDevice();
    this->initLogicalDevice();
    this->initCommandManager();
    this->initAllocator();
}

void GpuManager::destroy() {
    this->_logicalDevice->getHandle().waitIdle();

    this->_allocator->freeAll();

    this->_commandManager->destroy();
    this->_logicalDevice->destroy();

    this->_physicalDevice = nullptr;

    this->_instance.destroySurfaceKHR(this->_surface);
    this->_instance.destroy();
}
