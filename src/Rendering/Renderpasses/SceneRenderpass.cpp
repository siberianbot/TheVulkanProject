#include "SceneRenderpass.hpp"

#include "src/Engine.hpp"
#include "src/Scene/Object.hpp"
#include "src/Scene/Scene.hpp"
#include "src/Scene/Skybox.hpp"
#include "src/Resources/Vertex.hpp"
#include "src/Rendering/PhysicalDevice.hpp"
#include "src/Rendering/RenderingDevice.hpp"
#include "src/Rendering/RenderingObjectsFactory.hpp"
#include "src/Rendering/Swapchain.hpp"
#include "src/Rendering/Builders/RenderpassBuilder.hpp"
#include "src/Rendering/Builders/FramebuffersBuilder.hpp"
#include "src/Rendering/Builders/PipelineBuilder.hpp"
#include "src/Rendering/Builders/AttachmentBuilder.hpp"
#include "src/Rendering/Builders/SubpassBuilder.hpp"
#include "src/Rendering/Objects/BufferObject.hpp"
#include "src/Rendering/Objects/ImageObject.hpp"
#include "src/Rendering/Objects/ImageViewObject.hpp"
#include "src/Rendering/Objects/DescriptorSetObject.hpp"
#include "src/Rendering/Objects/RenderingLayoutObject.hpp"

SceneRenderpass::RenderData SceneRenderpass::getRenderData(Object *object) {
    auto it = this->_renderData.find(object);

    if (it != this->_renderData.end()) {
        return it->second;
    }

    ImageViewObject *textureView = this->_renderingObjectsFactory->createImageViewObject(object->texture()->texture,
                                                                                         VK_IMAGE_ASPECT_COLOR_BIT);

    RenderData renderData = {
            .textureView = textureView,
            .descriptorSet = this->_renderingLayoutObject->createMeshDataDescriptor(this->_textureSampler,
                                                                                    textureView->getHandle())
    };

    this->_renderData[object] = renderData;

    return renderData;
}

SceneRenderpass::SceneRenderpass(RenderingDevice *renderingDevice, Swapchain *swapchain,
                                 RenderingObjectsFactory *renderingObjectsFactory, Engine *engine)
        : RenderpassBase(renderingDevice, swapchain),
          _renderingObjectsFactory(renderingObjectsFactory),
          _engine(engine) {
    this->_skyboxTextureView = renderingObjectsFactory->createImageViewObject(
            this->_engine->scene()->skybox()->texture()->texture,
            VK_IMAGE_ASPECT_COLOR_BIT);

    this->_textureSampler = this->_renderingDevice->createSampler();
    this->_renderingLayoutObject = renderingObjectsFactory->createRenderingLayoutObject();
    this->_skyboxDescriptorSet = this->_renderingLayoutObject->createMeshDataDescriptor(
            this->_textureSampler, this->_skyboxTextureView->getHandle());
}

SceneRenderpass::~SceneRenderpass() {
    for (auto &it: this->_renderData) {
        delete it.second.descriptorSet;
        delete it.second.textureView;
    }

    delete this->_skyboxDescriptorSet;

    delete this->_renderingLayoutObject;
    this->_renderingDevice->destroySampler(this->_textureSampler);

    delete this->_skyboxTextureView;
}

void SceneRenderpass::recordCommands(VkCommandBuffer commandBuffer, VkRect2D renderArea,
                                     uint32_t frameIdx, uint32_t imageIdx) {
    VkPipelineLayout pipelineLayout = this->_renderingLayoutObject->getPipelineLayout();

    glm::mat4 projection = this->_engine->camera().getProjectionMatrix(renderArea.extent.width,
                                                                       renderArea.extent.height);
    projection[1][1] *= -1;

    VkDescriptorSet sceneDataDescriptorSet = this->_renderingLayoutObject->getSceneDataDescriptorSetObject()
            ->getDescriptorSet(frameIdx);

    const std::array<VkClearValue, 3> clearValues = {
            VkClearValue{.color = {{0, 0, 0, 1}}},
            VkClearValue{.color = {{0, 0, 0, 1}}},
            VkClearValue{.depthStencil = {1, 0}}
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

    VkDeviceSize offset = 0;

    {
        vkCmdBindPipeline(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, this->_skyboxPipeline);
        vkCmdBindDescriptorSets(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, pipelineLayout,
                                0, 1, &sceneDataDescriptorSet, 0, nullptr);

        VkDescriptorSet descriptorSet = this->_skyboxDescriptorSet->getDescriptorSet(frameIdx);

        MeshConstants constants = {
                .matrix = projection * this->_engine->camera().getViewMatrix(true) * glm::mat4(1)
        };
        vkCmdPushConstants(commandBuffer, pipelineLayout, VK_SHADER_STAGE_VERTEX_BIT, 0, sizeof(MeshConstants),
                           &constants);

        vkCmdBindDescriptorSets(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, pipelineLayout,
                                0, 1, &sceneDataDescriptorSet, 0, nullptr);

        vkCmdBindDescriptorSets(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, pipelineLayout,
                                1, 1, &descriptorSet, 0, nullptr);

        BufferObject *vertices = this->_engine->scene()->skybox()->mesh()->vertices;
        VkBuffer vertexBuffer = vertices->getHandle();
        vkCmdBindVertexBuffers(commandBuffer, 0, 1, &vertexBuffer, &offset);

        vkCmdDraw(commandBuffer, vertices->getSize(), 1, 0, 0);
    }

    vkCmdNextSubpass(commandBuffer, VK_SUBPASS_CONTENTS_INLINE);

    {
        vkCmdBindPipeline(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, this->_scenePipeline);
        vkCmdBindDescriptorSets(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, pipelineLayout,
                                0, 1, &sceneDataDescriptorSet, 0, nullptr);

        uint32_t idx = 0;
        for (Object *object: this->_engine->scene()->objects()) {
            RenderData renderData = getRenderData(object);

            VkDescriptorSet descriptorSet = renderData.descriptorSet->getDescriptorSet(frameIdx);

            MeshConstants constants = {
                    .matrix = projection * this->_engine->camera().getViewMatrix(false) * glm::mat4(1)
            };
            vkCmdPushConstants(commandBuffer, pipelineLayout, VK_SHADER_STAGE_VERTEX_BIT, 0, sizeof(MeshConstants),
                               &constants);

            VkBuffer vertexBuffer = object->mesh()->vertices->getHandle();
            vkCmdBindVertexBuffers(commandBuffer, 0, 1, &vertexBuffer, &offset);
            vkCmdBindIndexBuffer(commandBuffer, object->mesh()->indices->getHandle(), 0, VK_INDEX_TYPE_UINT32);

            vkCmdBindDescriptorSets(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, pipelineLayout,
                                    1, 1, &descriptorSet, 0, nullptr);

            vkCmdDrawIndexed(commandBuffer, object->mesh()->count, 1, 0, 0, idx++);
        }
    }

    vkCmdEndRenderPass(commandBuffer);
}

void SceneRenderpass::initRenderpass() {
    this->_renderpass = RenderpassBuilder(this->_renderingDevice)
            .addAttachment([this](AttachmentBuilder &builder) {
                builder
                        .clear()
                        .withFormat(this->_renderingDevice->getPhysicalDevice()->getColorFormat())
                        .withSamples(this->_renderingDevice->getPhysicalDevice()->getMsaaSamples())
                        .withFinalLayout(VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL);
            })
            .addAttachment([this](AttachmentBuilder &builder) {
                builder
                        .load()
                        .withFormat(this->_renderingDevice->getPhysicalDevice()->getColorFormat())
                        .withSamples(this->_renderingDevice->getPhysicalDevice()->getMsaaSamples())
                        .withInitialLayout(VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL)
                        .withFinalLayout(VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL);
            })
            .addAttachment([this](AttachmentBuilder &builder) {
                builder
                        .clear()
                        .withFormat(this->_renderingDevice->getPhysicalDevice()->getDepthFormat())
                        .withSamples(this->_renderingDevice->getPhysicalDevice()->getMsaaSamples())
                        .withFinalLayout(VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL);
            })
            .addAttachment([this](AttachmentBuilder &builder) {
                builder
                        .load()
                        .withFormat(this->_renderingDevice->getPhysicalDevice()->getColorFormat())
                        .withSamples(VK_SAMPLE_COUNT_1_BIT)
                        .withInitialLayout(VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL)
                        .withFinalLayout(VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL);
            })
            .addSubpass([](SubpassBuilder &builder) {
                builder.withColorAttachment(0, VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL);
            })
            .addSubpass([](SubpassBuilder &builder) {
                builder.withColorAttachment(1, VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL);
                builder.withDepthAttachment(2, VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL);
                builder.withResolveAttachment(3, VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL);
            })
            .addSubpassDependency(VK_SUBPASS_EXTERNAL, 0,
                                  VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT,
                                  VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT,
                                  0,
                                  VK_ACCESS_COLOR_ATTACHMENT_READ_BIT | VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT)
            .addSubpassDependency(0, 1,
                                  VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT,
                                  VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT,
                                  VK_ACCESS_COLOR_ATTACHMENT_READ_BIT | VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT,
                                  VK_ACCESS_COLOR_ATTACHMENT_READ_BIT | VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT)
            .build();

    this->_skyboxPipeline = PipelineBuilder(this->_renderingDevice, this->_renderpass,
                                           this->_renderingLayoutObject->getPipelineLayout())
            .addVertexShader(DEFAULT_VERTEX_SHADER)
            .addFragmentShader(DEFAULT_FRAGMENT_SHADER)
            .addBinding(0, sizeof(Vertex), VK_VERTEX_INPUT_RATE_VERTEX)
            .addAttribute(0, 0, offsetof(Vertex, pos), VK_FORMAT_R32G32B32_SFLOAT)
            .addAttribute(0, 1, offsetof(Vertex, color), VK_FORMAT_R32G32B32_SFLOAT)
            .addAttribute(0, 2, offsetof(Vertex, uv), VK_FORMAT_R32G32_SFLOAT)
            .forSubpass(0)
            .build();

    this->_scenePipeline = PipelineBuilder(this->_renderingDevice, this->_renderpass,
                                           this->_renderingLayoutObject->getPipelineLayout())
            .addVertexShader(DEFAULT_VERTEX_SHADER)
            .addFragmentShader(DEFAULT_FRAGMENT_SHADER)
            .addBinding(0, sizeof(Vertex), VK_VERTEX_INPUT_RATE_VERTEX)
            .addAttribute(0, 0, offsetof(Vertex, pos), VK_FORMAT_R32G32B32_SFLOAT)
            .addAttribute(0, 1, offsetof(Vertex, color), VK_FORMAT_R32G32B32_SFLOAT)
            .addAttribute(0, 2, offsetof(Vertex, uv), VK_FORMAT_R32G32_SFLOAT)
            .forSubpass(1)
            .build();
}

void SceneRenderpass::destroyRenderpass() {
    vkDestroyPipeline(this->_renderingDevice->getHandle(), this->_skyboxPipeline, nullptr);
    vkDestroyPipeline(this->_renderingDevice->getHandle(), this->_scenePipeline, nullptr);

    RenderpassBase::destroyRenderpass();
}

void SceneRenderpass::createFramebuffers() {
    VkExtent2D extent = this->_swapchain->getSwapchainExtent();
    VkSampleCountFlagBits samples = this->_renderingDevice->getPhysicalDevice()->getMsaaSamples();

    this->_colorImage = this->_renderingObjectsFactory->createImageObject(extent.width,
                                                                          extent.height,
                                                                          this->_renderingDevice->getPhysicalDevice()->getColorFormat(),
                                                                          VK_IMAGE_USAGE_TRANSIENT_ATTACHMENT_BIT |
                                                                          VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT,
                                                                          samples,
                                                                          VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);
    this->_colorImageView = this->_renderingObjectsFactory->createImageViewObject(this->_colorImage,
                                                                                  VK_IMAGE_ASPECT_COLOR_BIT);

    this->_depthImage = this->_renderingObjectsFactory->createImageObject(extent.width,
                                                                          extent.height,
                                                                          this->_renderingDevice->getPhysicalDevice()->getDepthFormat(),
                                                                          VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT,
                                                                          samples,
                                                                          VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);
    this->_depthImageView = this->_renderingObjectsFactory->createImageViewObject(this->_depthImage,
                                                                                  VK_IMAGE_ASPECT_DEPTH_BIT);

    this->_framebuffers = FramebuffersBuilder(this->_renderingDevice, this->_swapchain, this->_renderpass)
            .addAttachment(this->_colorImageView->getHandle())
            .addAttachment(this->_colorImageView->getHandle())
            .addAttachment(this->_depthImageView->getHandle())
            .build();
}

void SceneRenderpass::destroyFramebuffers() {
    RenderpassBase::destroyFramebuffers();

    delete this->_colorImageView;
    delete this->_colorImage;
    delete this->_depthImageView;
    delete this->_depthImage;
}
