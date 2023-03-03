#include "ImguiRenderpass.hpp"

#include <imgui.h>
#include <imgui_impl_vulkan.h>

#include "src/Rendering/PhysicalDevice.hpp"
#include "src/Rendering/RenderingDevice.hpp"
#include "src/Rendering/Swapchain.hpp"
#include "src/Rendering/CommandExecutor.hpp"
#include "src/Rendering/Builders/FramebufferBuilder.hpp"
#include "src/Rendering/Builders/RenderpassBuilder.hpp"
#include "src/Rendering/Builders/AttachmentBuilder.hpp"
#include "src/Rendering/Builders/SubpassBuilder.hpp"
#include "src/Rendering/Objects/ImageViewObject.hpp"

ImguiRenderpass::ImguiRenderpass(RenderingDevice *renderingDevice, Swapchain *swapchain, VkInstance instance,
                                 PhysicalDevice *physicalDevice, CommandExecutor *commandExecutor)
        : RenderpassBase(renderingDevice),
          _instance(instance),
          _physicalDevice(physicalDevice),
          _commandExecutor(commandExecutor),
          _swapchain(swapchain) {
    this->_descriptorPool = this->_renderingDevice->createDescriptorPool(
            {
                    {VK_DESCRIPTOR_TYPE_SAMPLER,                1000},
                    {VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, 1000},
                    {VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE,          1000},
                    {VK_DESCRIPTOR_TYPE_STORAGE_IMAGE,          1000},
                    {VK_DESCRIPTOR_TYPE_UNIFORM_TEXEL_BUFFER,   1000},
                    {VK_DESCRIPTOR_TYPE_STORAGE_TEXEL_BUFFER,   1000},
                    {VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER,         1000},
                    {VK_DESCRIPTOR_TYPE_STORAGE_BUFFER,         1000},
                    {VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC, 1000},
                    {VK_DESCRIPTOR_TYPE_STORAGE_BUFFER_DYNAMIC, 1000},
                    {VK_DESCRIPTOR_TYPE_INPUT_ATTACHMENT,       1000}
            }, 1000);
}

void ImguiRenderpass::recordCommands(VkCommandBuffer commandBuffer, VkRect2D renderArea, uint32_t frameIdx,
                                     uint32_t imageIdx) {
    ImGui::Render();
    ImDrawData *drawData = ImGui::GetDrawData();

    const VkRenderPassBeginInfo renderPassBeginInfo = {
            .sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO,
            .pNext = nullptr,
            .renderPass = this->_renderpass,
            .framebuffer = this->_framebuffers[imageIdx],
            .renderArea = renderArea,
            .clearValueCount = 0,
            .pClearValues = nullptr
    };

    vkCmdBeginRenderPass(commandBuffer, &renderPassBeginInfo, VK_SUBPASS_CONTENTS_INLINE);
    ImGui_ImplVulkan_RenderDrawData(drawData, commandBuffer);
    vkCmdEndRenderPass(commandBuffer);
}

void ImguiRenderpass::initRenderpass() {
    this->_renderpass = RenderpassBuilder(this->_renderingDevice)
            .addAttachment([](AttachmentBuilder &builder) {
                builder
                        .load()
                        .withInitialLayout(VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL)
                        .withFinalLayout(VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL);
            })
            .addSubpass([](SubpassBuilder &builder) {
                builder.withColorAttachment(0, VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL);
            })
            .addSubpassDependency(VK_SUBPASS_EXTERNAL, 0, VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT,
                                  VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT, 0,
                                  VK_ACCESS_COLOR_ATTACHMENT_READ_BIT | VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT)
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
    FramebufferBuilder builder = FramebufferBuilder(this->_renderingDevice, this->_renderpass)
            .withExtent(this->_swapchain->getSwapchainExtent())
            .addAttachment(nullptr);

    uint32_t count = this->_swapchain->getImageCount();
    this->_framebuffers.resize(count);
    
    for (uint32_t idx = 0; idx < count; idx++) {
        this->_framebuffers[idx] = builder
                .replaceAttachment(0, this->_swapchain->getSwapchainImageView(idx)->getHandle())
                .build();
    }

    ImGui_ImplVulkan_SetMinImageCount(this->_swapchain->getMinImageCount());
}

ImageViewObject *ImguiRenderpass::getResultImageView(uint32_t imageIdx) {
    return nullptr; // TODO
}
