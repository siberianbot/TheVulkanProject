#ifndef RENDERER_HPP
#define RENDERER_HPP

#include <vulkan/vulkan.hpp>
#include <GLFW/glfw3.h>
#include <optional>
#include <vector>
#include <string>
#include <algorithm>

#include "Mesh.hpp"
#include "Texture.hpp"
#include "Rendering/Renderpasses/RenderpassBase.hpp"
#include "RendererTypes.hpp"
#include "Rendering/Renderpasses/SceneRenderpass.hpp"
#include "Rendering/VulkanCommandExecutor.hpp"
#include "Rendering/VulkanPhysicalDevice.hpp"
#include "Rendering/RenderingObjectsFactory.hpp"
#include "Rendering/Swapchain.hpp"

#define GLM_FORCE_RADIANS
#define GLM_FORCE_DEPTH_ZERO_TO_ONE
#define GLM_ENABLE_EXPERIMENTAL
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

class Engine;

struct SyncObjectsGroup {
    FenceObject *fence;
    SemaphoreObject *imageAvailableSemaphore;
    SemaphoreObject *renderFinishedSemaphore;

    ~SyncObjectsGroup() {
        delete this->fence;
        delete this->imageAvailableSemaphore;
        delete this->renderFinishedSemaphore;
    }
};

class Renderer {
private:
    Engine *engine;

    VkInstance instance;
    VkSurfaceKHR surface;

    VulkanPhysicalDevice *_physicalDevice;
    RenderingDevice *_renderingDevice;
    RenderingObjectsFactory *_renderingObjectsFactory;
    VulkanCommandExecutor *_commandExecutor;
    Swapchain *_swapchain;
    std::array<SyncObjectsGroup *, MAX_INFLIGHT_FRAMES> _syncObjectsGroups;
    std::vector<RenderpassBase *> _renderpasses;
    SceneRenderpass *_sceneRenderpass;
    uint32_t _currentFrameIdx = 0;
    bool resizeRequested = false;

    void initInstance();
    void initSurface(GLFWwindow *window);

    void handleResize();

public:
    Renderer(Engine* engine);

    void init();
    void cleanup();

    void render();

    void requestResize(uint32_t width, uint32_t height);

    [[deprecated]] SceneRenderpass *sceneRenderpass() const { return this->_sceneRenderpass; };
};

#endif // RENDERER_HPP
