#include "ImguiRenderpass.hpp"

#include <imgui.h>
#include <imgui_impl_glfw.h>
#include <imgui_impl_vulkan.h>

#include <renderdoc_app.h>

#include "Rendering/PhysicalDevice.hpp"
#include "Rendering/RenderingDevice.hpp"
#include "Rendering/Swapchain.hpp"
#include "Rendering/CommandExecutor.hpp"
#include "Rendering/FramebuffersBuilder.hpp"
#include "Rendering/RenderpassBuilder.hpp"

ImguiRenderpass::ImguiRenderpass(RenderingDevice *renderingDevice, Swapchain *swapchain, VkInstance instance,
                                 PhysicalDevice *physicalDevice, GLFWwindow *window,
                                 CommandExecutor *commandExecutor)
        : RenderpassBase(renderingDevice, swapchain),
          _instance(instance),
          _physicalDevice(physicalDevice),
          _window(window),
          _commandExecutor(commandExecutor) {
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
    ImGui_ImplVulkan_NewFrame();
    ImGui_ImplGlfw_NewFrame();
    ImGui::NewFrame();

    ImGui::ShowDemoWindow();

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
            .noDepthAttachment()
            .load()
            .build();

    ImGui::CreateContext();
    ImGui::StyleColorsDark();

    ImGui_ImplGlfw_InitForVulkan(this->_window, true);
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
            .MSAASamples = this->_physicalDevice->getMsaaSamples(),
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
    ImGui::DestroyContext();

    this->_renderingDevice->destroyDescriptorPool(this->_descriptorPool);

    RenderpassBase::destroyRenderpass();
}

void ImguiRenderpass::createFramebuffers() {
    this->_framebuffers = FramebuffersBuilder(this->_renderingDevice, this->_swapchain, this->_renderpass)
            .build();

    ImGui_ImplVulkan_SetMinImageCount(this->_swapchain->getMinImageCount());
}
