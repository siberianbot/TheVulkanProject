#include "ImguiRenderpass.hpp"

#include <imgui.h>
#include <imgui_impl_vulkan.h>

#include "src/Rendering/Common.hpp"
#include "src/Rendering/PhysicalDevice.hpp"
#include "src/Rendering/RenderingDevice.hpp"
#include "src/Rendering/Swapchain.hpp"
#include "src/Rendering/CommandExecution.hpp"
#include "src/Rendering/CommandExecutor.hpp"
#include "src/Rendering/Builders/AttachmentBuilder.hpp"
#include "src/Rendering/Builders/DescriptorPoolBuilder.hpp"
#include "src/Rendering/Builders/FramebufferBuilder.hpp"
#include "src/Rendering/Builders/RenderpassBuilder.hpp"
#include "src/Rendering/Builders/SubpassBuilder.hpp"
#include "src/Rendering/Objects/ImageObject.hpp"
#include "src/Rendering/Objects/ImageViewObject.hpp"
#include "src/Rendering/Builders/ImageObjectBuilder.hpp"
#include "src/Rendering/Builders/ImageViewObjectBuilder.hpp"

ImguiRenderpass::ImguiRenderpass(const std::shared_ptr<RenderingDevice> &renderingDevice,
                                 const std::shared_ptr<VulkanObjectsAllocator> &vulkanObjectsAllocator,
                                 Swapchain *swapchain, VkInstance instance,
                                 PhysicalDevice *physicalDevice, CommandExecutor *commandExecutor)
        : RenderpassBase(renderingDevice),
          _vulkanObjectsAllocator(vulkanObjectsAllocator),
          _instance(instance),
          _physicalDevice(physicalDevice),
          _commandExecutor(commandExecutor),
          _swapchain(swapchain) {
    //
}

void ImguiRenderpass::recordCommands(VkCommandBuffer commandBuffer, VkRect2D renderArea, uint32_t frameIdx,
                                     uint32_t imageIdx) {
    ImGui::Render();
    ImDrawData *drawData = ImGui::GetDrawData();

    const std::array<VkClearValue, 1> clearValues = {
            VkClearValue{.color = {{0, 0, 0, 0}}}
    };

    const VkRenderPassBeginInfo renderPassBeginInfo = {
            .sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO,
            .pNext = nullptr,
            .renderPass = this->_renderpass,
            .framebuffer = this->_framebuffers[imageIdx],
            .renderArea = renderArea,
            .clearValueCount = static_cast<uint32_t>(clearValues.size()),
            .pClearValues = clearValues.data()
    };

    vkCmdBeginRenderPass(commandBuffer, &renderPassBeginInfo, VK_SUBPASS_CONTENTS_INLINE);
    ImGui_ImplVulkan_RenderDrawData(drawData, commandBuffer);
    vkCmdEndRenderPass(commandBuffer);
}

void ImguiRenderpass::initRenderpass() {
    this->_renderpass = RenderpassBuilder(this->_renderingDevice.get())
            .addAttachment([](AttachmentBuilder &builder) {
                builder
                        .clear()
                        .withFinalLayout(VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL);
            })
            .addSubpass([](SubpassBuilder &builder) {
                builder.withColorAttachment(0, VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL);
            })
            .addSubpassDependency(VK_SUBPASS_EXTERNAL, 0, VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT,
                                  VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT, 0,
                                  VK_ACCESS_COLOR_ATTACHMENT_READ_BIT | VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT)
            .build();

    this->_descriptorPool = DescriptorPoolBuilder(this->_renderingDevice.get())
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

    ImGui_ImplVulkan_InitInfo initInfo = {
            .Instance = this->_instance,
            .PhysicalDevice = this->_physicalDevice->getHandle(),
            .Device = this->_renderingDevice->getHandle(),
            .QueueFamily = this->_physicalDevice->getGraphicsQueueFamilyIdx(),
            .Queue = this->_renderingDevice->getGraphicsQueue(),
            .PipelineCache = nullptr,
            .DescriptorPool = this->_descriptorPool,
            .Subpass = 0,
            .MinImageCount = this->_swapchain->getMinImageCount(),
            .ImageCount = this->_swapchain->getImageCount(),
            .MSAASamples = VK_SAMPLE_COUNT_1_BIT,
            .Allocator = nullptr,
            .CheckVkResultFn = vkEnsure,
    };
    ImGui_ImplVulkan_Init(&initInfo, this->_renderpass);

    this->_commandExecutor->beginOneTimeExecution([](VkCommandBuffer cmdBuffer) {
                ImGui_ImplVulkan_CreateFontsTexture(cmdBuffer);
            })
            .submit(true);

    ImGui_ImplVulkan_DestroyFontUploadObjects();
}

void ImguiRenderpass::destroyRenderpass() {
    ImGui_ImplVulkan_Shutdown();

    this->_renderingDevice->destroyDescriptorPool(this->_descriptorPool);

    RenderpassBase::destroyRenderpass();
}

void ImguiRenderpass::createFramebuffers() {
    VkExtent2D extent = this->_swapchain->getSwapchainExtent();

    this->_resultImage = ImageObjectBuilder(this->_renderingDevice, this->_vulkanObjectsAllocator)
            .withExtent(extent.width, extent.height)
            .withFormat(this->_physicalDevice->getColorFormat())
            .withUsage(VK_IMAGE_USAGE_TRANSIENT_ATTACHMENT_BIT |
                       VK_IMAGE_USAGE_INPUT_ATTACHMENT_BIT |
                       VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT)
            .build();

    this->_resultImageView = ImageViewObjectBuilder(this->_vulkanObjectsAllocator)
            .fromImageObject(this->_resultImage)
            .withAspectFlags(VK_IMAGE_ASPECT_COLOR_BIT)
            .build();

    FramebufferBuilder builder = FramebufferBuilder(this->_renderingDevice.get(), this->_renderpass)
            .withExtent(extent)
            .addAttachment(this->_resultImageView->getHandle());

    uint32_t count = this->_swapchain->getImageCount();
    this->_framebuffers.resize(count);

    for (uint32_t idx = 0; idx < count; idx++) {
        this->_framebuffers[idx] = builder.build();
    }

    ImGui_ImplVulkan_SetMinImageCount(this->_swapchain->getMinImageCount());
}

void ImguiRenderpass::destroyFramebuffers() {
    RenderpassBase::destroyFramebuffers();

    this->_resultImageView->destroy();
    this->_resultImage->destroy();
}
