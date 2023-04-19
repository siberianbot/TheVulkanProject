#include "DebugUIRenderStage.hpp"

#include <imgui.h>
#include <imgui_impl_glfw.h>
#include <imgui_impl_vulkan.h>

#include "src/Rendering/CommandManager.hpp"
#include "src/Rendering/GpuManager.hpp"
#include "src/Rendering/Swapchain.hpp"
#include "src/Rendering/Proxies/CommandBufferProxy.hpp"
#include "src/Rendering/Proxies/LogicalDeviceProxy.hpp"
#include "src/Rendering/Proxies/PhysicalDeviceProxy.hpp"

DebugUIRenderStage::DebugUIRenderStage(const std::shared_ptr<GpuManager> &gpuManager)
        : _gpuManager(gpuManager) {
    //
}

void DebugUIRenderStage::init(const RenderStageInitContext &context) {
    auto physicalDevice = this->_gpuManager->getPhysicalDeviceProxy().lock();
    this->_logicalDevice = this->_gpuManager->getLogicalDeviceProxy().lock();
    auto commandManager = this->_gpuManager->getCommandManager().lock();

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

    auto attachment = vk::AttachmentDescription()
            .setLoadOp(vk::AttachmentLoadOp::eClear)
            .setStoreOp(vk::AttachmentStoreOp::eStore)
            .setInitialLayout(vk::ImageLayout::eUndefined)
            .setFinalLayout(vk::ImageLayout::ePresentSrcKHR)
            .setFormat(context.swapchain->getColorFormat());

    auto attachmentReference = vk::AttachmentReference()
            .setAttachment(0)
            .setLayout(vk::ImageLayout::eColorAttachmentOptimal);

    auto subpass = vk::SubpassDescription()
            .setColorAttachments(attachmentReference);

    auto subpassDependency = vk::SubpassDependency()
            .setSrcSubpass(VK_SUBPASS_EXTERNAL)
            .setDstSubpass(0)
            .setSrcStageMask(vk::PipelineStageFlagBits::eColorAttachmentOutput)
            .setDstStageMask(vk::PipelineStageFlagBits::eColorAttachmentOutput)
            .setSrcAccessMask(vk::AccessFlags())
            .setDstAccessMask(vk::AccessFlagBits::eColorAttachmentRead | vk::AccessFlagBits::eColorAttachmentWrite);

    auto renderpassCreateInfo = vk::RenderPassCreateInfo()
            .setAttachments(attachment)
            .setSubpasses(subpass)
            .setDependencies(subpassDependency);

    this->_renderpass = this->_logicalDevice->getHandle().createRenderPass(renderpassCreateInfo);

    ImGui_ImplVulkan_InitInfo initInfo = {
            .Instance = this->_gpuManager->getInstance(),
            .PhysicalDevice = physicalDevice->getHandle(),
            .Device = this->_logicalDevice->getHandle(),
            .QueueFamily = physicalDevice->getGraphicsQueueFamilyIdx(),
            .Queue = this->_logicalDevice->getGraphicsQueue(),
            .PipelineCache = nullptr,
            .DescriptorPool = this->_descriptorPool,
            .Subpass = 0,
            .MinImageCount = context.swapchain->getMinImageCount(),
            .ImageCount = context.swapchain->getImageCount(),
            .MSAASamples = VK_SAMPLE_COUNT_1_BIT,
            .Allocator = nullptr,
            .CheckVkResultFn = nullptr
    };

    ImGui_ImplVulkan_Init(&initInfo, this->_renderpass);

    auto commandBuffer = commandManager->createPrimaryBuffer();
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

    this->_framebuffers = std::vector<vk::Framebuffer>(context.swapchain->getImageCount());
    for (uint32_t idx = 0; idx < context.swapchain->getImageCount(); idx++) {
        auto framebufferCreateInfo = vk::FramebufferCreateInfo()
                .setRenderPass(this->_renderpass)
                .setAttachments(context.swapchain->getImageViews().at(idx))
                .setWidth(context.swapchain->getExtent().width)
                .setHeight(context.swapchain->getExtent().height)
                .setLayers(1);

        this->_framebuffers[idx] = this->_logicalDevice->getHandle().createFramebuffer(framebufferCreateInfo);
    }

    this->_renderArea = vk::Rect2D()
            .setOffset(vk::Offset2D(0, 0))
            .setExtent(context.swapchain->getExtent());

    this->_initialized = true;
}

void DebugUIRenderStage::destroy() {
    this->_initialized = false;

    ImGui_ImplVulkan_Shutdown();

    for (const auto &framebuffer: this->_framebuffers) {
        this->_logicalDevice->getHandle().destroy(framebuffer);
    }
    this->_framebuffers.clear();

    this->_logicalDevice->getHandle().destroy(this->_renderpass);
    this->_logicalDevice->getHandle().destroy(this->_descriptorPool);

    this->_logicalDevice = nullptr;
}

void DebugUIRenderStage::draw(uint32_t imageIdx, const vk::CommandBuffer &commandBuffer) {
    ImGui_ImplVulkan_NewFrame();
    ImGui_ImplGlfw_NewFrame();
    ImGui::NewFrame();

    ImGui::ShowDemoWindow(); // TODO: render actual UI

    auto renderpassBeginInfo = vk::RenderPassBeginInfo()
            .setRenderPass(this->_renderpass)
            .setClearValues(vk::ClearValue().setColor(vk::ClearColorValue()))
            .setFramebuffer(this->_framebuffers[imageIdx])
            .setRenderArea(this->_renderArea);

    commandBuffer.beginRenderPass(renderpassBeginInfo, vk::SubpassContents::eInline);

    ImGui::Render();
    ImDrawData *drawData = ImGui::GetDrawData();
    ImGui_ImplVulkan_RenderDrawData(drawData, commandBuffer);

    commandBuffer.endRenderPass();
}
