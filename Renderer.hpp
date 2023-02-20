#ifndef RENDERER_HPP
#define RENDERER_HPP

#include <array>
#include <optional>
#include <vector>

#include <vulkan/vulkan.hpp>
#include <GLFW/glfw3.h>

#include "VulkanConstants.hpp"

class Engine;
class VulkanPhysicalDevice;
class RenderingDevice;
class RenderingObjectsFactory;
class VulkanCommandExecutor;
class Swapchain;
class RenderpassBase;
class SceneRenderpass;
class FenceObject;
class SemaphoreObject;

class Renderer {
private:
    struct SyncObjectsGroup {
        FenceObject *fence;
        SemaphoreObject *imageAvailableSemaphore;
        SemaphoreObject *renderFinishedSemaphore;

        ~SyncObjectsGroup();
    };

    Engine *_engine;

    uint32_t _currentFrameIdx = 0;

    VkInstance _instance = VK_NULL_HANDLE;
    VkSurfaceKHR _surface = VK_NULL_HANDLE;
    VulkanPhysicalDevice *_physicalDevice;
    RenderingDevice *_renderingDevice;
    RenderingObjectsFactory *_renderingObjectsFactory;
    VulkanCommandExecutor *_commandExecutor;
    Swapchain *_swapchain;
    std::array<SyncObjectsGroup *, MAX_INFLIGHT_FRAMES> _syncObjectsGroups;
    std::vector<RenderpassBase *> _renderpasses;
    SceneRenderpass *_sceneRenderpass;

    static VKAPI_ATTR VkBool32 VKAPI_CALL debugCallback(VkDebugUtilsMessageSeverityFlagBitsEXT messageSeverity,
                                                        VkDebugUtilsMessageTypeFlagsEXT messageType,
                                                        const VkDebugUtilsMessengerCallbackDataEXT *pCallbackData,
                                                        void *pUserData);

    VkInstance createInstance();
    VkSurfaceKHR createSurface(GLFWwindow *window);

    void handleResize();

public:
    Renderer(Engine *engine);

    void init();
    void cleanup();

    void render();

    void requestResize();

    [[deprecated]] SceneRenderpass *sceneRenderpass() const { return this->_sceneRenderpass; };
};

#endif // RENDERER_HPP
