#include "SwapchainPresentRenderpass.hpp"

#include "src/Engine/Engine.hpp"
#include "src/Rendering/PhysicalDevice.hpp"
#include "src/Rendering/RenderingDevice.hpp"
#include "src/Rendering/Swapchain.hpp"
#include "src/Rendering/Builders/AttachmentBuilder.hpp"
#include "src/Rendering/Builders/DescriptorPoolBuilder.hpp"
#include "src/Rendering/Builders/DescriptorSetLayoutBuilder.hpp"
#include "src/Rendering/Builders/FramebufferBuilder.hpp"
#include "src/Rendering/Builders/PipelineBuilder.hpp"
#include "src/Rendering/Builders/PipelineLayoutBuilder.hpp"
#include "src/Rendering/Builders/RenderpassBuilder.hpp"
#include "src/Rendering/Builders/SubpassBuilder.hpp"
#include "src/Rendering/Objects/DescriptorSetObject.hpp"
#include "src/Rendering/Objects/ImageViewObject.hpp"
#include "src/Resources/ResourceManager.hpp"
#include "src/Resources/ShaderResource.hpp"

SwapchainPresentRenderpass::SwapchainPresentRenderpass(const std::shared_ptr<RenderingDevice> &renderingDevice,
                                                       Swapchain *swapchain,
                                                       Engine *engine)
        : RenderpassBase(renderingDevice),
          _swapchain(swapchain),
          _engine(engine) {
    //
}

void SwapchainPresentRenderpass::recordCommands(VkCommandBuffer commandBuffer, VkRect2D renderArea, uint32_t frameIdx,
                                                uint32_t imageIdx) {
    const std::array<VkClearValue, 1> clearValues = {
            VkClearValue{.color = {{0, 0, 0, 1}}}
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

    VkViewport viewport = {
            .x = 0,
            .y = 0,
            .width = (float) renderArea.extent.width,
            .height = (float) renderArea.extent.height,
            .minDepth = 0.0f,
            .maxDepth = 1.0f
    };
    vkCmdSetViewport(commandBuffer, 0, 1, &viewport);

    VkRect2D scissor = {
            .offset = {0, 0},
            .extent = renderArea.extent
    };
    vkCmdSetScissor(commandBuffer, 0, 1, &scissor);

    vkCmdBindPipeline(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, this->_pipeline);

    VkDescriptorSet descriptor = this->_descriptorSets[imageIdx]->getHandle();
    vkCmdBindDescriptorSets(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, this->_pipelineLayout, 0, 1,
                            &descriptor, 0, nullptr);

    vkCmdDraw(commandBuffer, 3, 1, 0, 0);

    vkCmdEndRenderPass(commandBuffer);
}

void SwapchainPresentRenderpass::addInputRenderpass(RenderpassBase *renderpass) {
    this->_inputRenderpasses.push_back(renderpass);
}

void SwapchainPresentRenderpass::initRenderpass() {
    VkFormat colorFormat = this->_renderingDevice->getPhysicalDevice()->getColorFormat();

    DescriptorPoolBuilder descriptorPoolBuilder = DescriptorPoolBuilder(this->_renderingDevice.get());
    DescriptorSetLayoutBuilder descriptorSetLayoutBuilder = DescriptorSetLayoutBuilder(this->_renderingDevice.get());
    // TODO: destructor problem, cleanses subpass data before actual build
    RenderpassBuilder renderpassBuilder = RenderpassBuilder(this->_renderingDevice.get());

    renderpassBuilder.addAttachment([&](AttachmentBuilder &builder) {
                builder
                        .clear()
                        .withFormat(colorFormat)
                        .withSamples(VK_SAMPLE_COUNT_1_BIT)
                        .withInitialLayout(VK_IMAGE_LAYOUT_UNDEFINED)
                        .withFinalLayout(VK_IMAGE_LAYOUT_PRESENT_SRC_KHR);
            })
            .addSubpass([&](SubpassBuilder &builder) {
                builder.withColorAttachment(0, VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL);

                for (uint32_t passIdx = 0; passIdx < this->_inputRenderpasses.size(); passIdx++) {
                    builder.withInputAttachment(passIdx + 1, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL);
                }
            })
            .addSubpassDependency(VK_SUBPASS_EXTERNAL, 0,
                                  VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT,
                                  VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT,
                                  0,
                                  VK_ACCESS_COLOR_ATTACHMENT_READ_BIT | VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT);

    for (uint32_t passIdx = 0; passIdx < this->_inputRenderpasses.size(); passIdx++) {
        renderpassBuilder.addAttachment([&](AttachmentBuilder &builder) {
            builder
                    .load()
                    .withFormat(colorFormat)
                    .withSamples(VK_SAMPLE_COUNT_1_BIT)
                    .withInitialLayout(VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL)
                    .withFinalLayout(VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL);
        });

        descriptorPoolBuilder.forType(VK_DESCRIPTOR_TYPE_INPUT_ATTACHMENT, this->_swapchain->getImageCount());
        descriptorSetLayoutBuilder.withBinding(passIdx, VK_DESCRIPTOR_TYPE_INPUT_ATTACHMENT,
                                               VK_SHADER_STAGE_FRAGMENT_BIT);
    }

    this->_renderpass = renderpassBuilder.build();
    this->_descriptorPool = descriptorPoolBuilder.build();
    this->_descriptorSetLayout = descriptorSetLayoutBuilder.build();

    this->_pipelineLayout = PipelineLayoutBuilder(this->_renderingDevice.get())
            .withDescriptorSetLayout(this->_descriptorSetLayout)
            .build();

    ShaderResource *vertexShader = this->_engine->resourceManager()->loadShader("composition_vert");
    ShaderResource *fragmentShader = this->_engine->resourceManager()->loadShader("composition_frag");

    this->_pipeline = PipelineBuilder(this->_renderingDevice.get(), this->_renderpass, this->_pipelineLayout)
            .addVertexShader(vertexShader->shader())
            .addFragmentShader(fragmentShader->shader())
            .withCullMode(VK_CULL_MODE_FRONT_BIT)
            .build();

    vertexShader->unload();
    fragmentShader->unload();
}

void SwapchainPresentRenderpass::destroyRenderpass() {
    this->_renderingDevice->destroyPipeline(this->_pipeline);
    this->_renderingDevice->destroyPipelineLayout(this->_pipelineLayout);
    this->_renderingDevice->destroyDescriptorSetLayout(this->_descriptorSetLayout);
    this->_renderingDevice->destroyDescriptorPool(this->_descriptorPool);

    RenderpassBase::destroyRenderpass();
}

void SwapchainPresentRenderpass::createFramebuffers() {
    VkExtent2D extent = this->_swapchain->getSwapchainExtent();
    uint32_t imagesCount = this->_swapchain->getImageCount();
    uint32_t passesCount = this->_inputRenderpasses.size();

    this->_framebuffers.resize(imagesCount);
    for (uint32_t imageIdx = 0; imageIdx < imagesCount; imageIdx++) {
        FramebufferBuilder builder = FramebufferBuilder(this->_renderingDevice.get(), this->_renderpass)
                .withExtent(extent)
                .addAttachment(this->_swapchain->getSwapchainImageView(imageIdx)->getHandle());

        for (uint32_t passIdx = 0; passIdx < passesCount; passIdx++) {
            builder.addAttachment(this->_inputRenderpasses[passIdx]->getResultImageView(imageIdx)->getHandle());
        }

        this->_framebuffers[imageIdx] = builder.build();
    }

    this->_descriptorSets.resize(imagesCount);
    for (uint32_t imageIdx = 0; imageIdx < imagesCount; imageIdx++) {
        this->_descriptorSets[imageIdx] = DescriptorSetObject::create(this->_renderingDevice,
                                                                      this->_descriptorPool,
                                                                      this->_descriptorSetLayout);
    }

    for (uint32_t passIdx = 0; passIdx < passesCount; passIdx++) {
        for (uint32_t imageIdx = 0; imageIdx < imagesCount; imageIdx++) {
            VkDescriptorImageInfo imageInfo = {
                    .sampler = VK_NULL_HANDLE,
                    .imageView = this->_inputRenderpasses[passIdx]->getResultImageView(imageIdx)->getHandle(),
                    .imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL
            };

            std::vector<VkWriteDescriptorSet> writes = {
                    VkWriteDescriptorSet{
                            .sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET,
                            .pNext = nullptr,
                            .dstSet = this->_descriptorSets[imageIdx]->getHandle(),
                            .dstBinding = passIdx,
                            .dstArrayElement = 0,
                            .descriptorCount = 1,
                            .descriptorType = VK_DESCRIPTOR_TYPE_INPUT_ATTACHMENT,
                            .pImageInfo = &imageInfo,
                            .pBufferInfo = nullptr,
                            .pTexelBufferView = nullptr
                    }
            };

            vkUpdateDescriptorSets(this->_renderingDevice->getHandle(), static_cast<uint32_t>(writes.size()),
                                   writes.data(), 0, nullptr);
        }
    }
}

void SwapchainPresentRenderpass::destroyFramebuffers() {
    for (std::shared_ptr<DescriptorSetObject> &descriptorSet: this->_descriptorSets) {
        descriptorSet->destroy();
    }

    RenderpassBase::destroyFramebuffers();
}
