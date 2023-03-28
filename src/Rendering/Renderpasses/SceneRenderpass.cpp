#include "SceneRenderpass.hpp"

#include "src/Engine/VarCollection.hpp"
#include "src/Engine/Vars.hpp"
#include "src/Rendering/PhysicalDevice.hpp"
#include "src/Rendering/RenderingLayoutsManager.hpp"
#include "src/Rendering/Swapchain.hpp"
#include "src/Rendering/VulkanObjectsAllocator.hpp"
#include "src/Rendering/Builders/BufferObjectBuilder.hpp"
#include "src/Rendering/Builders/FramebufferBuilder.hpp"
#include "src/Rendering/Builders/RenderpassBuilder.hpp"
#include "src/Rendering/Builders/AttachmentBuilder.hpp"
#include "src/Rendering/Builders/SubpassBuilder.hpp"
#include "src/Rendering/Builders/PipelineBuilder.hpp"
#include "src/Rendering/Builders/SpecializationInfoBuilder.hpp"
#include "src/Rendering/Builders/SamplerBuilder.hpp"
#include "src/Rendering/Objects/BufferObject.hpp"
#include "src/Rendering/Objects/DescriptorSetObject.hpp"
#include "src/Rendering/Objects/ImageViewObject.hpp"
#include "src/Rendering/Types/MeshConstants.hpp"
#include "src/Resources/ResourceManager.hpp"
#include "src/Resources/ShaderResource.hpp"
#include "src/Types/Vertex.hpp"

VkFramebuffer SceneRenderpass::createFramebuffer(const std::shared_ptr<ImageViewObject> &imageView, VkExtent2D extent) {
    return FramebufferBuilder(this->_renderingDevice.get(), this->_renderpass)
            .withExtent(extent)
            .addAttachment(this->_albedoImageView.lock()->getHandle())
            .addAttachment(this->_positionImageView.lock()->getHandle())
            .addAttachment(this->_normalImageView.lock()->getHandle())
            .addAttachment(this->_specularImageView.lock()->getHandle())
            .addAttachment(this->_depthImageView.lock()->getHandle())
            .addAttachment(this->_compositionImageView.lock()->getHandle())
            .addAttachment(imageView->getHandle())
            .build();
}

SceneRenderpass::SceneRenderpass(const std::shared_ptr<RenderingDevice> &renderingDevice,
                                 const std::shared_ptr<VarCollection> &vars,
                                 const std::shared_ptr<PhysicalDevice> &physicalDevice,
                                 const std::shared_ptr<RenderingLayoutsManager> &renderingLayoutsManager,
                                 const std::shared_ptr<VulkanObjectsAllocator> &vulkanObjectsAllocator,
                                 const std::shared_ptr<Swapchain> &swapchain,
                                 const std::shared_ptr<ResourceManager> &resourceManager)
        : RenderpassBase(renderingDevice,
                         {
                                 VkClearValue{.color = {{0, 0, 0, 0}}},
                                 VkClearValue{.color = {{0, 0, 0, 0}}},
                                 VkClearValue{.color = {{0, 0, 0, 0}}},
                                 VkClearValue{.color = {{0, 0, 0, 0}}},
                                 VkClearValue{.depthStencil = {1, 0}},
                                 VkClearValue{.color = {{0, 0, 0, 0}}},
                                 VkClearValue{.color = {{0, 0, 0, 0}}},
                         }),
          _vars(vars),
          _physicalDevice(physicalDevice),
          _renderingLayoutsManager(renderingLayoutsManager),
          _vulkanObjectsAllocator(vulkanObjectsAllocator),
          _swapchain(swapchain),
          _resourceManager(resourceManager) {
    //
}

void SceneRenderpass::record(VkCommandBuffer commandBuffer, uint32_t frameIdx,
                             const SceneData &sceneData,
                             const CameraData &cameraData,
                             const std::vector<ShadowData> &shadowData,
                             const std::vector<LightData> &lightData,
                             const std::vector<ModelData> &models) {
    *this->_sceneData = sceneData;
    *this->_cameraData = cameraData;

    for (uint32_t idx = 0; idx < this->_sceneData->shadowCount; idx++) {
        this->_shadowData[idx] = shadowData[idx];
    }

    for (uint32_t idx = 0; idx < this->_sceneData->lightCount; idx++) {
        this->_lightData[idx] = lightData[idx];
    }

    VkDeviceSize offset = 0;
    MeshConstants meshConstants;
    VkDescriptorSet descriptorSet;

    vkCmdBindPipeline(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, this->_modelPipeline);

    for (uint32_t idx = 0; idx < models.size(); idx++) {
        ModelData model = models[idx];

        meshConstants.matrix = cameraData.matrixAll * model.model;
        meshConstants.model = model.model;
        meshConstants.modelRotation = model.modelRotation;
        vkCmdPushConstants(commandBuffer, this->_renderingLayoutsManager->modelPipelineLayout(),
                           VK_SHADER_STAGE_VERTEX_BIT, 0, sizeof(MeshConstants), &meshConstants);

        descriptorSet = model.descriptorSet->getHandle();
        vkCmdBindDescriptorSets(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS,
                                this->_renderingLayoutsManager->modelPipelineLayout(),
                                0, 1, &descriptorSet, 0, nullptr);

        VkBuffer vertexBuffer = model.vertices->getHandle();
        vkCmdBindVertexBuffers(commandBuffer, 0, 1, &vertexBuffer, &offset);
        vkCmdBindIndexBuffer(commandBuffer, model.indices->getHandle(), 0, VK_INDEX_TYPE_UINT32);

        vkCmdDrawIndexed(commandBuffer, model.count, 1, 0, 0, idx);
    }

    vkCmdNextSubpass(commandBuffer, VK_SUBPASS_CONTENTS_INLINE);

    vkCmdBindPipeline(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, this->_compositionPipeline);

    descriptorSet = this->_compositionDescriptorSets[frameIdx]->getHandle();
    vkCmdBindDescriptorSets(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS,
                            this->_renderingLayoutsManager->compositionPipelineLayout(),
                            0, 1, &descriptorSet, 0, nullptr);

    vkCmdDraw(commandBuffer, 3, 1, 0, 0);
}

void SceneRenderpass::beginRenderpass(VkCommandBuffer commandBuffer) {
    RenderpassBase::beginRenderpass(commandBuffer);

    const VkViewport viewport = {
            .x = 0,
            .y = 0,
            .width = (float) this->_targetRenderArea.extent.width,
            .height = (float) this->_targetRenderArea.extent.height,
            .minDepth = 0.0f,
            .maxDepth = 1.0f
    };

    vkCmdSetViewport(commandBuffer, 0, 1, &viewport);
    vkCmdSetScissor(commandBuffer, 0, 1, &this->_targetRenderArea);
}

void SceneRenderpass::initRenderpass() {
    this->_sceneConstants.lightCount = this->_vars->getOrDefault(RENDERING_SCENE_STAGE_LIGHT_COUNT, 128);
    this->_sceneConstants.shadowCount = this->_vars->getOrDefault(RENDERING_SCENE_STAGE_SHADOW_MAP_COUNT, 32);

    VkFormat colorFormat = this->_physicalDevice->getColorFormat();
    VkSampleCountFlagBits samples = this->_physicalDevice->getMsaaSamples();

    this->_textureSampler = SamplerBuilder(this->_vulkanObjectsAllocator)
            .enableAnisotropy(this->_physicalDevice->getMaxSamplerAnisotropy())
            .build();

    this->_shadowMapSampler = SamplerBuilder(this->_vulkanObjectsAllocator)
            .withBorderColor(VK_BORDER_COLOR_FLOAT_OPAQUE_WHITE)
            .withAddressMode(VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_BORDER)
            .build();

    this->_renderpass = RenderpassBuilder(this->_renderingDevice.get())
            .addAttachment([&](AttachmentBuilder &builder) {
                // 0. Albedo
                builder
                        .clear()
                        .withFormat(VK_FORMAT_R8G8B8A8_UNORM)
                        .withSamples(samples)
                        .withFinalLayout(VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL);
            })
            .addAttachment([&](AttachmentBuilder &builder) {
                // 1. Position
                builder
                        .clear()
                        .withFormat(VK_FORMAT_R16G16B16A16_SFLOAT)
                        .withSamples(samples)
                        .withFinalLayout(VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL);
            })
            .addAttachment([&](AttachmentBuilder &builder) {
                // 2. Normals
                builder
                        .clear()
                        .withFormat(VK_FORMAT_R16G16B16A16_SFLOAT)
                        .withSamples(samples)
                        .withFinalLayout(VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL);
            })
            .addAttachment([&](AttachmentBuilder &builder) {
                // 3. Specular
                builder
                        .clear()
                        .withFormat(VK_FORMAT_R8_UNORM)
                        .withSamples(samples)
                        .withFinalLayout(VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL);
            })
            .addAttachment([&](AttachmentBuilder &builder) {
                // 4. Depth
                builder
                        .clear()
                        .withFormat(this->_physicalDevice->getDepthFormat())
                        .withSamples(samples)
                        .withFinalLayout(VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL);
            })
            .addAttachment([&](AttachmentBuilder &builder) {
                // 5. Composition
                builder
                        .clear()
                        .withFormat(colorFormat)
                        .withSamples(samples)
                        .withFinalLayout(VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL);
            })
            .addAttachment([&](AttachmentBuilder &builder) {
                // 6. Resolve
                builder
                        .clear()
                        .withFormat(colorFormat)
                        .withFinalLayout(VK_IMAGE_LAYOUT_PRESENT_SRC_KHR);
            })
            .addSubpass([](SubpassBuilder &builder) {
                builder
                        .withColorAttachment(0, VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL)
                        .withColorAttachment(1, VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL)
                        .withColorAttachment(2, VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL)
                        .withColorAttachment(3, VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL)
                        .withDepthAttachment(4, VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL);
            })
            .addSubpass([](SubpassBuilder &builder) {
                builder
                        .withInputAttachment(0, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL)
                        .withInputAttachment(1, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL)
                        .withInputAttachment(2, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL)
                        .withInputAttachment(3, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL)
                        .withColorAttachment(5, VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL)
                        .withResolveAttachment(6, VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL);
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

    this->_modelPipeline = PipelineBuilder(this->_vulkanObjectsAllocator, this->_renderpass,
                                           this->_renderingLayoutsManager->modelPipelineLayout())
            .addVertexShader(this->_resourceManager->loadShader("shaders/scene-model.vert")->shader())
            .addFragmentShader(this->_resourceManager->loadShader("shaders/scene-model.frag")->shader())
            .addBinding(0, sizeof(Vertex), VK_VERTEX_INPUT_RATE_VERTEX)
            .addAttribute(0, 0, offsetof(Vertex, pos), VK_FORMAT_R32G32B32_SFLOAT)
            .addAttribute(0, 1, offsetof(Vertex, normal), VK_FORMAT_R32G32B32_SFLOAT)
            .addAttribute(0, 2, offsetof(Vertex, color), VK_FORMAT_R32G32B32_SFLOAT)
            .addAttribute(0, 3, offsetof(Vertex, uv), VK_FORMAT_R32G32_SFLOAT)
            .withRasterizationSamples(this->_physicalDevice->getMsaaSamples())
            .withColorBlendAttachmentCount(4)
            .forSubpass(0)
            .build();

    this->_compositionPipeline = PipelineBuilder(this->_vulkanObjectsAllocator, this->_renderpass,
                                                 this->_renderingLayoutsManager->compositionPipelineLayout())
            .addVertexShader(this->_resourceManager->loadShader("shaders/passthrough.vert")->shader())
            .addFragmentShader(this->_resourceManager->loadShader("shaders/scene-composition.frag")->shader())
            .withFragmentShaderSpecialization([&](SpecializationInfoBuilder &builder) {
                builder
                        .withSize(sizeof(SceneConstants))
                        .withEntry(0, offsetof(SceneConstants, shadowCount), sizeof(uint32_t))
                        .withEntry(1, offsetof(SceneConstants, lightCount), sizeof(uint32_t))
                        .withData(&this->_sceneConstants);
            })
            .withRasterizationSamples(this->_physicalDevice->getMsaaSamples())
            .withCullMode(VK_CULL_MODE_FRONT_BIT)
            .forSubpass(1)
            .build();

    this->_shadowDataBuffer = BufferObjectBuilder(this->_renderingDevice, this->_vulkanObjectsAllocator)
            .withSize(sizeof(ShadowData) * this->_sceneConstants.shadowCount)
            .withUsage(VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT)
            .hostAvailable()
            .build();

    this->_shadowData = reinterpret_cast<ShadowData *>(this->_shadowDataBuffer->map());

    this->_lightDataBuffer = BufferObjectBuilder(this->_renderingDevice, this->_vulkanObjectsAllocator)
            .withSize(sizeof(LightData) * this->_sceneConstants.lightCount)
            .withUsage(VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT)
            .hostAvailable()
            .build();

    this->_lightData = reinterpret_cast<LightData *>(this->_lightDataBuffer->map());

    this->_cameraDataBuffer = BufferObjectBuilder(this->_renderingDevice, this->_vulkanObjectsAllocator)
            .withSize(sizeof(CameraData))
            .withUsage(VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT)
            .hostAvailable()
            .build();

    this->_cameraData = reinterpret_cast<CameraData *>(this->_cameraDataBuffer->map());

    this->_sceneDataBuffer = BufferObjectBuilder(this->_renderingDevice, this->_vulkanObjectsAllocator)
            .withSize(sizeof(SceneData))
            .withUsage(VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT)
            .hostAvailable()
            .build();

    this->_sceneData = reinterpret_cast<SceneData *>(this->_sceneDataBuffer->map());

    for (uint32_t imageIdx = 0; imageIdx < MAX_INFLIGHT_FRAMES; imageIdx++) {
        this->_compositionDescriptorSets[imageIdx] = DescriptorSetObject::create(
                this->_renderingDevice,
                this->_renderingLayoutsManager->descriptorPool(),
                this->_renderingLayoutsManager->compositionDescriptorSetLayout());

        this->_compositionDescriptorSets[imageIdx]->updateWithBuffer(
                5, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER,
                this->_shadowDataBuffer, 0, sizeof(ShadowData) * this->_sceneConstants.shadowCount);

        this->_compositionDescriptorSets[imageIdx]->updateWithBuffer(
                6, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER,
                this->_lightDataBuffer, 0, sizeof(LightData) * this->_sceneConstants.lightCount);

        this->_compositionDescriptorSets[imageIdx]->updateWithBuffer(
                7, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, this->_cameraDataBuffer, 0, sizeof(CameraData));

        this->_compositionDescriptorSets[imageIdx]->updateWithBuffer(
                8, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, this->_sceneDataBuffer, 0, sizeof(SceneData));
    }
}

void SceneRenderpass::destroyRenderpass() {
    for (uint32_t imageIdx = 0; imageIdx < MAX_INFLIGHT_FRAMES; imageIdx++) {
        this->_compositionDescriptorSets[imageIdx]->destroy();
    }

    this->_sceneDataBuffer->destroy();
    this->_cameraDataBuffer->destroy();
    this->_lightDataBuffer->destroy();
    this->_shadowDataBuffer->destroy();

    this->_vulkanObjectsAllocator->destroyPipeline(this->_compositionPipeline);
    this->_vulkanObjectsAllocator->destroyPipeline(this->_modelPipeline);

    RenderpassBase::destroyRenderpass();

    this->_vulkanObjectsAllocator->destroySampler(this->_textureSampler);
    this->_vulkanObjectsAllocator->destroySampler(this->_shadowMapSampler);
}

void SceneRenderpass::setGBufferImageViews(const std::shared_ptr<ImageViewObject> &albedoImageView,
                                           const std::shared_ptr<ImageViewObject> &positionImageView,
                                           const std::shared_ptr<ImageViewObject> &normalImageView,
                                           const std::shared_ptr<ImageViewObject> &specularImageView,
                                           const std::shared_ptr<ImageViewObject> &depthImageView,
                                           const std::shared_ptr<ImageViewObject> &compositionImageView) {
    this->_albedoImageView = albedoImageView;
    this->_positionImageView = positionImageView;
    this->_normalImageView = normalImageView;
    this->_specularImageView = specularImageView;
    this->_depthImageView = depthImageView;
    this->_compositionImageView = compositionImageView;

    for (uint32_t imageIdx = 0; imageIdx < MAX_INFLIGHT_FRAMES; imageIdx++) {
        this->_compositionDescriptorSets[imageIdx]->updateWithImageView(
                0, VK_DESCRIPTOR_TYPE_INPUT_ATTACHMENT, albedoImageView,
                VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL);

        this->_compositionDescriptorSets[imageIdx]->updateWithImageView(
                1, VK_DESCRIPTOR_TYPE_INPUT_ATTACHMENT, positionImageView,
                VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL);

        this->_compositionDescriptorSets[imageIdx]->updateWithImageView(
                2, VK_DESCRIPTOR_TYPE_INPUT_ATTACHMENT, normalImageView,
                VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL);

        this->_compositionDescriptorSets[imageIdx]->updateWithImageView(
                3, VK_DESCRIPTOR_TYPE_INPUT_ATTACHMENT, specularImageView,
                VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL);
    }
}

void SceneRenderpass::setShadowMapArrayView(const std::shared_ptr<ImageViewObject> &shadowMapArrayView) {
    for (uint32_t imageIdx = 0; imageIdx < MAX_INFLIGHT_FRAMES; imageIdx++) {
        this->_compositionDescriptorSets[imageIdx]->updateWithImageView(
                4, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, shadowMapArrayView,
                VK_IMAGE_LAYOUT_DEPTH_STENCIL_READ_ONLY_OPTIMAL, this->_shadowMapSampler);
    }
}
