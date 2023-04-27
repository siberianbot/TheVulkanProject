#include "DebugUIRenderStage.hpp"

#include <fmt/core.h>
#include <imgui.h>
#include <imgui_impl_glfw.h>
#include <imgui_impl_vulkan.h>

#include "src/Debug/DebugUIRoot.hpp"
#include "src/Engine/EngineError.hpp"
#include "src/Rendering/CommandManager.hpp"
#include "src/Rendering/GpuManager.hpp"
#include "src/Rendering/Swapchain.hpp"
#include "src/Rendering/Proxies/CommandBufferProxy.hpp"
#include "src/Rendering/Proxies/LogicalDeviceProxy.hpp"
#include "src/Rendering/Proxies/PhysicalDeviceProxy.hpp"

DebugUIRenderStage::DebugUIRenderStage(const std::shared_ptr<GpuManager> &gpuManager,
                                       const std::shared_ptr<DebugUIRoot> &debugUIRoot)
        : _gpuManager(gpuManager),
          _debugUIRoot(debugUIRoot) {
    //
}

void DebugUIRenderStage::init() {
    if (this->_gpuManager->getPhysicalDeviceProxy().expired() ||
        this->_gpuManager->getLogicalDeviceProxy().expired() ||
        this->_gpuManager->getCommandManager().expired()) {
        throw EngineError("GPU manager is not initialized");
    }

    this->_physicalDevice = this->_gpuManager->getPhysicalDeviceProxy().lock();
    this->_logicalDevice = this->_gpuManager->getLogicalDeviceProxy().lock();
    this->_commandManager = this->_gpuManager->getCommandManager().lock();

    auto poolSizes = {
            vk::DescriptorPoolSize(vk::DescriptorType::eSampler, 1024),
            vk::DescriptorPoolSize(vk::DescriptorType::eCombinedImageSampler, 1024),
            vk::DescriptorPoolSize(vk::DescriptorType::eSampledImage, 1024),
            vk::DescriptorPoolSize(vk::DescriptorType::eStorageImage, 1024),
            vk::DescriptorPoolSize(vk::DescriptorType::eUniformTexelBuffer, 1024),
            vk::DescriptorPoolSize(vk::DescriptorType::eStorageTexelBuffer, 1024),
            vk::DescriptorPoolSize(vk::DescriptorType::eUniformBuffer, 1024),
            vk::DescriptorPoolSize(vk::DescriptorType::eStorageBuffer, 1024),
            vk::DescriptorPoolSize(vk::DescriptorType::eUniformBufferDynamic, 1024),
            vk::DescriptorPoolSize(vk::DescriptorType::eStorageBufferDynamic, 1024),
            vk::DescriptorPoolSize(vk::DescriptorType::eInputAttachment, 1024),
    };

    auto descriptorPoolCreateInfo = vk::DescriptorPoolCreateInfo()
            .setMaxSets(1024) // TODO: vars
            .setPoolSizes(poolSizes);

    this->_descriptorPool = this->_logicalDevice->getHandle().createDescriptorPool(descriptorPoolCreateInfo);
}

void DebugUIRenderStage::destroy() {
    this->_logicalDevice->getHandle().destroy(this->_descriptorPool);

    this->_commandManager = nullptr;
    this->_logicalDevice = nullptr;
    this->_physicalDevice = nullptr;
}

void DebugUIRenderStage::onGraphCreate(const std::shared_ptr<Swapchain> swapchain,
                                       const vk::RenderPass &renderPass) {
    ImGui_ImplVulkan_InitInfo initInfo = {
            .Instance = this->_gpuManager->getInstance(),
            .PhysicalDevice = this->_physicalDevice->getHandle(),
            .Device = this->_logicalDevice->getHandle(),
            .QueueFamily = this->_physicalDevice->getGraphicsQueueFamilyIdx(),
            .Queue = this->_logicalDevice->getGraphicsQueue(),
            .PipelineCache = nullptr,
            .DescriptorPool = this->_descriptorPool,
            .Subpass = 0,
            .MinImageCount = swapchain->getMinImageCount(),
            .ImageCount = swapchain->getImageCount(),
            .MSAASamples = VK_SAMPLE_COUNT_1_BIT,
            .Allocator = nullptr,
            .CheckVkResultFn = [](VkResult result) {
                if (result != VK_SUCCESS) {
                    throw EngineError("Vulkan assertion failed");
                }
            }
    };

    ImGui_ImplVulkan_Init(&initInfo, renderPass);

    auto commandBuffer = this->_commandManager->createPrimaryBuffer();
    auto commandBufferBeginInfo = vk::CommandBufferBeginInfo();

    commandBuffer->getHandle().begin(commandBufferBeginInfo);
    ImGui_ImplVulkan_CreateFontsTexture(commandBuffer->getHandle());
    commandBuffer->getHandle().end();

    auto submitInfo = vk::SubmitInfo()
            .setCommandBuffers(commandBuffer->getHandle());
    this->_logicalDevice->getGraphicsQueue().submit(submitInfo);
    this->_logicalDevice->getGraphicsQueue().waitIdle();

    commandBuffer->destroy();

    ImGui_ImplVulkan_DestroyFontUploadObjects();
}

void DebugUIRenderStage::onGraphDestroy() {
    ImGui_ImplVulkan_Shutdown();
}

void DebugUIRenderStage::onPassExecute(const RenderPassRef &passRef, const vk::CommandBuffer &commandBuffer) {
    if (passRef != "DebugUI") {
        throw EngineError(fmt::format("Unknown pass {0}", passRef));
    }

    ImGui_ImplVulkan_NewFrame();
    ImGui_ImplGlfw_NewFrame();
    ImGui::NewFrame();

    this->_debugUIRoot->render();

    ImGui::Render();
    ImDrawData *drawData = ImGui::GetDrawData();
    ImGui_ImplVulkan_RenderDrawData(drawData, commandBuffer);
}

RenderSubgraph DebugUIRenderStage::asSubgraph() {
    return RenderSubgraph{
            .attachments = {
                    {
                            "Swapchain",
                            RenderAttachment{
                                    .idx = 0,
                                    .targetRef = "Swapchain",
                                    .loadOp = vk::AttachmentLoadOp::eClear,
                                    .storeOp = vk::AttachmentStoreOp::eStore,
                                    .initialLayout = vk::ImageLayout::eUndefined,
                                    .finalLayout = vk::ImageLayout::ePresentSrcKHR
                            }
                    }
            },
            .passes = {
                    {
                            "DebugUI",
                            RenderPass{
                                    .idx = 0,
                                    .inputRefs = {},
                                    .colorRefs = {
                                            "Swapchain"
                                    },
                                    .depthRef = std::nullopt,
                                    .dependencies = {}
                            }
                    }
            },
            .firstPass = "DebugUI",
            .next = {}
    };
}
