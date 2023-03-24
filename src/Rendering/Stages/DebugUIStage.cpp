#include "DebugUIStage.hpp"

#include <imgui_impl_vulkan.h>

#include "src/Events/EventQueue.hpp"
#include "src/Rendering/Common.hpp"
#include "src/Rendering/CommandExecution.hpp"
#include "src/Rendering/CommandExecutor.hpp"
#include "src/Rendering/PhysicalDevice.hpp"
#include "src/Rendering/RenderingDevice.hpp"
#include "src/Rendering/RenderingManager.hpp"
#include "src/Rendering/Swapchain.hpp"
#include "src/Rendering/Builders/DescriptorPoolBuilder.hpp"
#include "src/Rendering/Renderpasses/DebugUIRenderpass.hpp"

DebugUIStage::DebugUIStage(const std::shared_ptr<EventQueue> &eventQueue,
                           const std::shared_ptr<RenderingManager> &renderingManager)
        : _eventQueue(eventQueue),
          _renderingManager(renderingManager) {
    //
}

void DebugUIStage::init() {
    this->_descriptorPool = DescriptorPoolBuilder(this->_renderingManager->renderingDevice().get())
            .forType(VK_DESCRIPTOR_TYPE_SAMPLER)
            .forType(VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER)
            .forType(VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE)
            .forType(VK_DESCRIPTOR_TYPE_STORAGE_IMAGE)
            .forType(VK_DESCRIPTOR_TYPE_UNIFORM_TEXEL_BUFFER)
            .forType(VK_DESCRIPTOR_TYPE_STORAGE_TEXEL_BUFFER)
            .forType(VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER)
            .forType(VK_DESCRIPTOR_TYPE_STORAGE_BUFFER)
            .forType(VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC)
            .forType(VK_DESCRIPTOR_TYPE_STORAGE_BUFFER_DYNAMIC)
            .forType(VK_DESCRIPTOR_TYPE_INPUT_ATTACHMENT)
            .build();

    this->_debugUIRenderpass = std::make_unique<DebugUIRenderpass>(this->_renderingManager->renderingDevice());
    this->_debugUIRenderpass->initRenderpass();

    ImGui_ImplVulkan_InitInfo initInfo = {
            .Instance = this->_renderingManager->instance(),
            .PhysicalDevice = this->_renderingManager->physicalDevice()->getHandle(),
            .Device = this->_renderingManager->renderingDevice()->getHandle(),
            .QueueFamily = this->_renderingManager->physicalDevice()->getGraphicsQueueFamilyIdx(),
            .Queue = this->_renderingManager->renderingDevice()->getGraphicsQueue(),
            .PipelineCache = nullptr,
            .DescriptorPool = this->_descriptorPool,
            .Subpass = 0,
            .MinImageCount = this->_renderingManager->swapchain()->getMinImageCount(),
            .ImageCount =  this->_renderingManager->swapchain()->getImageCount(),
            .MSAASamples = VK_SAMPLE_COUNT_1_BIT,
            .Allocator = nullptr,
            .CheckVkResultFn = vkEnsure,
    };
    ImGui_ImplVulkan_Init(&initInfo, this->_debugUIRenderpass->getHandle());

    this->_renderingManager->commandExecutor()->beginOneTimeExecution([](VkCommandBuffer cmdBuffer) {
                ImGui_ImplVulkan_CreateFontsTexture(cmdBuffer);
            })
            .submit(true);

    ImGui_ImplVulkan_DestroyFontUploadObjects();

    this->_eventQueue->addHandler([&](const Event &event) {
        if (event.type != RESIZE_WINDOW_EVENT) {
            return;
        }

        ImGui_ImplVulkan_SetMinImageCount(this->_renderingManager->swapchain()->getMinImageCount());

        this->_debugUIRenderpass->destroyFramebuffers();
    });
}

void DebugUIStage::destroy() {
    ImGui_ImplVulkan_Shutdown();

    this->_debugUIRenderpass->destroyFramebuffers();
    this->_debugUIRenderpass->destroyRenderpass();

    vkDestroyDescriptorPool(this->_renderingManager->renderingDevice()->getHandle(), this->_descriptorPool, nullptr);
}

void DebugUIStage::record(VkCommandBuffer commandBuffer, uint32_t frameIdx, uint32_t imageIdx) {
    const VkRect2D renderArea = {
            .offset = {0, 0},
            .extent = this->_renderingManager->swapchain()->getSwapchainExtent()
    };

    this->_debugUIRenderpass->setTargetImageView(this->_renderingManager->swapchain()->getSwapchainImageView(imageIdx),
                                                 renderArea);

    this->_debugUIRenderpass->beginRenderpass(commandBuffer);
    this->_debugUIRenderpass->record(commandBuffer);
    this->_debugUIRenderpass->endRenderpass(commandBuffer);
}
