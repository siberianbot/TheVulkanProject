#ifndef RENDERING_RENDERER_HPP
#define RENDERING_RENDERER_HPP

#include <array>
#include <optional>
#include <vector>

#include <vulkan/vulkan.hpp>
#include <GLFW/glfw3.h>

#include "Rendering/Common.hpp"

class Engine;
class PhysicalDevice;
class RenderingDevice;
class CommandExecutor;
class Swapchain;
class RenderingObjectsFactory;
class FenceObject;
class SemaphoreObject;
class RenderpassBase;
class SceneRenderpass;

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
    PhysicalDevice *_physicalDevice;
    RenderingDevice *_renderingDevice;
    RenderingObjectsFactory *_renderingObjectsFactory;
    CommandExecutor *_commandExecutor;
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

#endif // RENDERING_RENDERER_HPP
