#include "SceneRenderpass.hpp"

#include "src/Engine.hpp"
#include "src/Events/EventQueue.hpp"
#include "src/Scene/Light.hpp"
#include "src/Scene/Object.hpp"
#include "src/Scene/Scene.hpp"
#include "src/Scene/Skybox.hpp"
#include "src/Resources/Vertex.hpp"
#include "src/Rendering/PhysicalDevice.hpp"
#include "src/Rendering/RenderingDevice.hpp"
#include "src/Rendering/RenderingObjectsFactory.hpp"
#include "src/Rendering/Swapchain.hpp"
#include "src/Rendering/Builders/RenderpassBuilder.hpp"
#include "src/Rendering/Builders/FramebufferBuilder.hpp"
#include "src/Rendering/Builders/PipelineBuilder.hpp"
#include "src/Rendering/Builders/AttachmentBuilder.hpp"
#include "src/Rendering/Builders/SubpassBuilder.hpp"
#include "src/Rendering/Builders/DescriptorPoolBuilder.hpp"
#include "src/Rendering/Builders/DescriptorSetLayoutBuilder.hpp"
#include "src/Rendering/Builders/PipelineLayoutBuilder.hpp"
#include "src/Rendering/Objects/BufferObject.hpp"
#include "src/Rendering/Objects/ImageObject.hpp"
#include "src/Rendering/Objects/ImageViewObject.hpp"
#include "src/Rendering/Objects/DescriptorSetObject.hpp"

SceneRenderpass::RenderData SceneRenderpass::getRenderData(Object *object) {
    auto it = this->_renderData.find(object);

    if (it != this->_renderData.end()) {
        return it->second;
    }

    ImageObject *targetImage = object->texture() == nullptr
                               ? this->_engine->defaultTextureResource()->texture
                               : object->texture()->texture;

    ImageViewObject *textureView = this->_renderingObjectsFactory->createImageViewObject(targetImage,
                                                                                         VK_IMAGE_VIEW_TYPE_2D_ARRAY,
                                                                                         VK_IMAGE_ASPECT_COLOR_BIT);

    RenderData renderData = {
            .textureView = textureView,
            .descriptorSet = this->createTextureDescriptorSetFor(textureView)
    };

    this->_renderData[object] = renderData;

    return renderData;
}

DescriptorSetObject *SceneRenderpass::createTextureDescriptorSetFor(ImageViewObject *imageViewObject) {
    DescriptorSetObject *descriptorSetObject = this->_renderingObjectsFactory->createDescriptorSetObject(
            this->_descriptorPool, this->_objectDescriptorSetLayout, MAX_INFLIGHT_FRAMES);

    VkDescriptorImageInfo imageInfo = {
            .sampler = this->_textureSampler,
            .imageView = imageViewObject->getHandle(),
            .imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL
    };

    std::vector<VkWriteDescriptorSet> writes;
    for (uint32_t idx = 0; idx < MAX_INFLIGHT_FRAMES; idx++) {
        writes.push_back(VkWriteDescriptorSet{
                .sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET,
                .pNext = nullptr,
                .dstSet = descriptorSetObject->getDescriptorSet(idx),
                .dstBinding = 0,
                .dstArrayElement = 0,
                .descriptorCount = 1,
                .descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER,
                .pImageInfo = &imageInfo,
                .pBufferInfo = nullptr,
                .pTexelBufferView = nullptr
        });
    }

    this->_renderingDevice->updateDescriptorSets(writes);

    return descriptorSetObject;
}

void SceneRenderpass::initSkyboxPipeline() {
    this->_skyboxPipelineLayout = PipelineLayoutBuilder(this->_renderingDevice)
            .withDescriptorSetLayout(this->_objectDescriptorSetLayout)
            .withPushConstant(VK_SHADER_STAGE_VERTEX_BIT, 0, sizeof(MeshConstants))
            .build();

    this->_skyboxPipeline = PipelineBuilder(this->_renderingDevice, this->_renderpass, this->_skyboxPipelineLayout)
            .addVertexShader(DEFAULT_VERTEX_SHADER)
            .addFragmentShader(SKYBOX_FRAGMENT_SHADER)
            .addBinding(0, sizeof(Vertex), VK_VERTEX_INPUT_RATE_VERTEX)
            .addAttribute(0, 0, offsetof(Vertex, pos), VK_FORMAT_R32G32B32_SFLOAT)
            .addAttribute(0, 1, offsetof(Vertex, normal), VK_FORMAT_R32G32B32_SFLOAT)
            .addAttribute(0, 2, offsetof(Vertex, color), VK_FORMAT_R32G32B32_SFLOAT)
            .addAttribute(0, 3, offsetof(Vertex, uv), VK_FORMAT_R32G32_SFLOAT)
            .withRasterizationSamples(this->_renderingDevice->getPhysicalDevice()->getMsaaSamples())
            .forSubpass(0)
            .build();

    this->_skyboxTextureView = this->_renderingObjectsFactory->createImageViewObject(
            this->_engine->scene()->skybox()->texture()->texture,
            VK_IMAGE_VIEW_TYPE_CUBE,
            VK_IMAGE_ASPECT_COLOR_BIT);

    this->_skyboxDescriptorSet = this->createTextureDescriptorSetFor(
            this->_skyboxTextureView);
}

void SceneRenderpass::destroySkyboxPipeline() {
    delete this->_skyboxDescriptorSet;
    delete this->_skyboxTextureView;

    this->_renderingDevice->destroyPipeline(this->_skyboxPipeline);
    this->_renderingDevice->destroyPipelineLayout(this->_skyboxPipelineLayout);
}

void SceneRenderpass::initScenePipeline() {
    this->_scenePipelineLayout = PipelineLayoutBuilder(this->_renderingDevice)
            .withDescriptorSetLayout(this->_objectDescriptorSetLayout)
            .withPushConstant(VK_SHADER_STAGE_VERTEX_BIT, 0, sizeof(MeshConstants))
            .build();

    this->_scenePipeline = PipelineBuilder(this->_renderingDevice, this->_renderpass, this->_scenePipelineLayout)
            .addVertexShader(DEFAULT_VERTEX_SHADER)
            .addFragmentShader(DEFAULT_FRAGMENT_SHADER)
            .addBinding(0, sizeof(Vertex), VK_VERTEX_INPUT_RATE_VERTEX)
            .addAttribute(0, 0, offsetof(Vertex, pos), VK_FORMAT_R32G32B32_SFLOAT)
            .addAttribute(0, 1, offsetof(Vertex, normal), VK_FORMAT_R32G32B32_SFLOAT)
            .addAttribute(0, 2, offsetof(Vertex, color), VK_FORMAT_R32G32B32_SFLOAT)
            .addAttribute(0, 3, offsetof(Vertex, uv), VK_FORMAT_R32G32_SFLOAT)
            .withRasterizationSamples(this->_renderingDevice->getPhysicalDevice()->getMsaaSamples())
            .withColorBlendAttachmentCount(4)
            .forSubpass(1)
            .build();
}

void SceneRenderpass::destroyScenePipeline() {
    for (const auto &item: this->_renderData) {
        delete item.second.textureView;
        delete item.second.descriptorSet;
    }

    this->_renderData.clear();

    this->_renderingDevice->destroyPipeline(this->_scenePipeline);
    this->_renderingDevice->destroyPipelineLayout(this->_scenePipelineLayout);
}

void SceneRenderpass::initCompositionPipeline() {
    this->_compositionGBufferDescriptorSetLayout = DescriptorSetLayoutBuilder(this->_renderingDevice)
            .withBinding(0, VK_DESCRIPTOR_TYPE_INPUT_ATTACHMENT, VK_SHADER_STAGE_FRAGMENT_BIT)
            .withBinding(1, VK_DESCRIPTOR_TYPE_INPUT_ATTACHMENT, VK_SHADER_STAGE_FRAGMENT_BIT)
            .withBinding(2, VK_DESCRIPTOR_TYPE_INPUT_ATTACHMENT, VK_SHADER_STAGE_FRAGMENT_BIT)
            .withBinding(3, VK_DESCRIPTOR_TYPE_INPUT_ATTACHMENT, VK_SHADER_STAGE_FRAGMENT_BIT)
            .withBinding(4, VK_DESCRIPTOR_TYPE_INPUT_ATTACHMENT, VK_SHADER_STAGE_FRAGMENT_BIT)
            .build();

    this->_compositionSceneDataDescriptorSetLayout = DescriptorSetLayoutBuilder(this->_renderingDevice)
            .withBinding(0, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, VK_SHADER_STAGE_FRAGMENT_BIT)
            .withBinding(1, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, VK_SHADER_STAGE_FRAGMENT_BIT, MAX_NUM_LIGHTS)
            .build();

    this->_compositionSceneDataBuffer = this->_renderingObjectsFactory->createBufferObject(
            sizeof(SceneData), VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT,
            VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT);

    this->_compositionSceneData = reinterpret_cast<SceneData *>(this->_compositionSceneDataBuffer->map());

    this->_compositionSceneDataDescriptorSet = this->_renderingObjectsFactory->createDescriptorSetObject(
            this->_descriptorPool, this->_compositionSceneDataDescriptorSetLayout, MAX_INFLIGHT_FRAMES);

    for (uint32_t frameIdx = 0; frameIdx < MAX_INFLIGHT_FRAMES; frameIdx++) {
        VkDescriptorBufferInfo bufferInfo = {
                .buffer = this->_compositionSceneDataBuffer->getHandle(),
                .offset = 0,
                .range = sizeof(SceneData)
        };

        std::vector<VkDescriptorImageInfo> imageInfo(MAX_NUM_LIGHTS);
        for (uint32_t idx = 0; idx < MAX_NUM_LIGHTS; idx++) {
            imageInfo[idx] = {
                    .sampler = this->_textureSampler,
                    .imageView = this->_shadowRenderpasses[0]->getResultImageView(idx)->getHandle(),
                    .imageLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_READ_ONLY_OPTIMAL
            };
        }

        std::vector<VkWriteDescriptorSet> writes = {
                VkWriteDescriptorSet{
                        .sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET,
                        .pNext = nullptr,
                        .dstSet = this->_compositionSceneDataDescriptorSet->getDescriptorSet(frameIdx),
                        .dstBinding = 0,
                        .dstArrayElement = 0,
                        .descriptorCount = 1,
                        .descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER,
                        .pImageInfo = nullptr,
                        .pBufferInfo = &bufferInfo,
                        .pTexelBufferView = nullptr
                },
                VkWriteDescriptorSet{
                        .sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET,
                        .pNext = nullptr,
                        .dstSet = this->_compositionSceneDataDescriptorSet->getDescriptorSet(frameIdx),
                        .dstBinding = 1,
                        .dstArrayElement = 0,
                        .descriptorCount = static_cast<uint32_t>(imageInfo.size()),
                        .descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER,
                        .pImageInfo = imageInfo.data(),
                        .pBufferInfo = nullptr,
                        .pTexelBufferView = nullptr
                }
        };

        this->_renderingDevice->updateDescriptorSets(writes);
    }

    this->_compositionPipelineLayout = PipelineLayoutBuilder(this->_renderingDevice)
            .withDescriptorSetLayout(this->_compositionGBufferDescriptorSetLayout)
            .withDescriptorSetLayout(this->_compositionSceneDataDescriptorSetLayout)
            .build();

    this->_compositionPipeline = PipelineBuilder(this->_renderingDevice, this->_renderpass,
                                                 this->_compositionPipelineLayout)
            .addVertexShader("data/shaders/composition.vert.spv")
            .addFragmentShader("data/shaders/scene_composition.frag.spv")
            .withRasterizationSamples(this->_renderingDevice->getPhysicalDevice()->getMsaaSamples())
            .withCullMode(VK_CULL_MODE_FRONT_BIT)
            .forSubpass(2)
            .build();
}

void SceneRenderpass::destroyCompositionPipeline() {
    this->_renderingDevice->destroyPipeline(this->_compositionPipeline);
    this->_renderingDevice->destroyPipelineLayout(this->_compositionPipelineLayout);
    this->_renderingDevice->destroyDescriptorSetLayout(this->_compositionSceneDataDescriptorSetLayout);
    this->_renderingDevice->destroyDescriptorSetLayout(this->_compositionGBufferDescriptorSetLayout);
    delete this->_compositionSceneDataDescriptorSet;
    delete this->_compositionSceneDataBuffer;
}

SceneRenderpass::SceneRenderpass(RenderingDevice *renderingDevice, Swapchain *swapchain,
                                 RenderingObjectsFactory *renderingObjectsFactory, Engine *engine)
        : RenderpassBase(renderingDevice),
          _renderingObjectsFactory(renderingObjectsFactory),
          _engine(engine),
          _swapchain(swapchain) {
    this->_engine->eventQueue()->addHandler([this](const Event &event) {
        if (event.type != OBJECT_DESTROYED_EVENT) {
            return;
        }

        auto it = this->_renderData.find(event.object);
        if (it != this->_renderData.end()) {
            delete it->second.descriptorSet;
            delete it->second.textureView;

            this->_renderData.erase(it);
        }
    });
}

void SceneRenderpass::recordCommands(VkCommandBuffer commandBuffer, VkRect2D renderArea,
                                     uint32_t frameIdx, uint32_t imageIdx) {
    // PREPARE SCENE DATA
    {
        glm::vec3 cameraPosition = this->_engine->camera().position();

        std::vector<Light *> lights(this->_engine->scene()->lights().size());
        std::copy_if(this->_engine->scene()->lights().begin(), this->_engine->scene()->lights().end(), lights.begin(),
                     [&](Light *l) {
                         return glm::distance(cameraPosition, l->position()) < 100;
                     });

        this->_compositionSceneData->numLights = std::min(MAX_NUM_LIGHTS, (int) lights.size());
        for (int idx = 0; idx < this->_compositionSceneData->numLights; idx++) {
            Light *light = lights[idx];

            this->_compositionSceneData->lights[idx] = LightData{
                    .projection = light->getProjectionMatrix() * light->getViewMatrix(),
                    .position = light->position(),
                    .color = light->color(),
                    .radius = light->radius()
            };
        }

        this->_compositionSceneData->cameraPosition = cameraPosition;
    }

    glm::mat4 projection = this->_engine->camera().getProjectionMatrix(renderArea.extent.width,
                                                                       renderArea.extent.height);
    projection[1][1] *= -1;

    const std::array<VkClearValue, 8> clearValues = {
            VkClearValue{.color = {{0, 0, 0, 0}}}, // 0
            VkClearValue{.color = {{0, 0, 0, 0}}}, // 1
            VkClearValue{.color = {{0, 0, 0, 0}}}, // 2
            VkClearValue{.color = {{0, 0, 0, 0}}}, // 3
            VkClearValue{.color = {{0, 0, 0, 0}}}, // 4
            VkClearValue{.depthStencil = {1, 0}},  // 5
            VkClearValue{.color = {{0, 0, 0, 0}}}, // 6
            VkClearValue{.color = {{0, 0, 0, 0}}}  // 7
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

    // SKYBOX
    {
        vkCmdBindPipeline(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, this->_skyboxPipeline);

        MeshConstants constants = {
                .matrix = projection * this->_engine->camera().getViewMatrix(true) * glm::mat4(1),
                .model = glm::mat4(1)
        };
        vkCmdPushConstants(commandBuffer, this->_skyboxPipelineLayout, VK_SHADER_STAGE_VERTEX_BIT, 0,
                           sizeof(MeshConstants), &constants);

        VkDescriptorSet descriptorSet = this->_skyboxDescriptorSet->getDescriptorSet(frameIdx);
        vkCmdBindDescriptorSets(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, this->_skyboxPipelineLayout,
                                0, 1, &descriptorSet, 0, nullptr);

        BufferObject *vertices = this->_engine->scene()->skybox()->mesh()->vertices;
        VkBuffer vertexBuffer = vertices->getHandle();
        vkCmdBindVertexBuffers(commandBuffer, 0, 1, &vertexBuffer, &offset);

        vkCmdDraw(commandBuffer, vertices->getSize(), 1, 0, 0);
    }

    vkCmdNextSubpass(commandBuffer, VK_SUBPASS_CONTENTS_INLINE);

    // SCENE
    {
        vkCmdBindPipeline(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, this->_scenePipeline);

        glm::mat4 view = this->_engine->camera().getViewMatrix(false);

        uint32_t idx = 0;
        for (Object *object: this->_engine->scene()->objects()) {
            RenderData renderData = getRenderData(object);

            glm::mat4 model = object->getModelMatrix();
            MeshConstants constants = {
                    .matrix = projection * view * model,
                    .model = model
            };
            vkCmdPushConstants(commandBuffer, this->_scenePipelineLayout, VK_SHADER_STAGE_VERTEX_BIT, 0,
                               sizeof(MeshConstants), &constants);

            VkBuffer vertexBuffer = object->mesh()->vertices->getHandle();
            vkCmdBindVertexBuffers(commandBuffer, 0, 1, &vertexBuffer, &offset);

            if (object->mesh()->indices != nullptr) {
                vkCmdBindIndexBuffer(commandBuffer, object->mesh()->indices->getHandle(), 0, VK_INDEX_TYPE_UINT32);
            }

            VkDescriptorSet descriptorSet = renderData.descriptorSet->getDescriptorSet(frameIdx);
            vkCmdBindDescriptorSets(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, this->_scenePipelineLayout,
                                    0, 1, &descriptorSet, 0, nullptr);

            if (object->mesh()->indices != nullptr) {
                vkCmdDrawIndexed(commandBuffer, object->mesh()->count, 1, 0, 0, idx++);
            } else {
                vkCmdDraw(commandBuffer, object->mesh()->vertices->getSize(), 1, 0, 0);
            }
        }
    }

    vkCmdNextSubpass(commandBuffer, VK_SUBPASS_CONTENTS_INLINE);

    // COMPOSITION
    {
        vkCmdBindPipeline(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, this->_compositionPipeline);

        VkDescriptorSet gBufferDescriptor = this->_compositionGBufferDescriptorSet->getDescriptorSet(frameIdx);
        vkCmdBindDescriptorSets(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, this->_compositionPipelineLayout,
                                0, 1, &gBufferDescriptor, 0, nullptr);

        VkDescriptorSet sceneDataDescriptor = this->_compositionSceneDataDescriptorSet->getDescriptorSet(frameIdx);
        vkCmdBindDescriptorSets(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, this->_compositionPipelineLayout,
                                1, 1, &sceneDataDescriptor, 0, nullptr);

        vkCmdDraw(commandBuffer, 3, 1, 0, 0);
    }

    vkCmdEndRenderPass(commandBuffer);
}

void SceneRenderpass::initRenderpass() {
    VkFormat format = this->_renderingDevice->getPhysicalDevice()->getColorFormat();
    VkSampleCountFlagBits samples = this->_renderingDevice->getPhysicalDevice()->getMsaaSamples();
    this->_renderpass = RenderpassBuilder(this->_renderingDevice)
            .addAttachment([&](AttachmentBuilder &builder) {
                // 0: skybox
                builder
                        .clear()
                        .withFormat(format)
                        .withSamples(samples)
                        .withFinalLayout(VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL);
            })
            .addAttachment([&](AttachmentBuilder &builder) {
                // 1: scene albedo
                builder
                        .clear()
                        .withFormat(VK_FORMAT_R8G8B8A8_UNORM)
                        .withSamples(samples)
                        .withFinalLayout(VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL);
            })
            .addAttachment([&](AttachmentBuilder &builder) {
                // 2: scene position
                builder
                        .clear()
                        .withFormat(VK_FORMAT_R16G16B16A16_SFLOAT)
                        .withSamples(samples)
                        .withFinalLayout(VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL);
            })
            .addAttachment([&](AttachmentBuilder &builder) {
                // 3: scene normals
                builder
                        .clear()
                        .withFormat(VK_FORMAT_R16G16B16A16_SFLOAT)
                        .withSamples(samples)
                        .withFinalLayout(VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL);
            })
            .addAttachment([&](AttachmentBuilder &builder) {
                // 4: scene specular
                builder
                        .clear()
                        .withFormat(VK_FORMAT_R8G8B8A8_UNORM)
                        .withSamples(samples)
                        .withFinalLayout(VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL);
            })
            .addAttachment([&](AttachmentBuilder &builder) {
                // 5: scene depth
                builder
                        .clear()
                        .withFormat(this->_renderingDevice->getPhysicalDevice()->getDepthFormat())
                        .withSamples(samples)
                        .withFinalLayout(VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL);
            })
            .addAttachment([&](AttachmentBuilder &builder) {
                // 6: composition
                builder
                        .clear()
                        .withFormat(format)
                        .withSamples(samples)
                        .withFinalLayout(VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL);
            })
            .addAttachment([&](AttachmentBuilder &builder) {
                // 7: resolve
                builder
                        .clear()
                        .withFormat(format)
                        .withFinalLayout(VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL);
            })
            .addSubpass([](SubpassBuilder &builder) {
                builder.withColorAttachment(0, VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL);
            })
            .addSubpass([](SubpassBuilder &builder) {
                builder
                        .withColorAttachment(1, VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL)
                        .withColorAttachment(2, VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL)
                        .withColorAttachment(3, VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL)
                        .withColorAttachment(4, VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL)
                        .withDepthAttachment(5, VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL);
            })
            .addSubpass([](SubpassBuilder &builder) {
                builder
                        .withInputAttachment(0, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL)
                        .withInputAttachment(1, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL)
                        .withInputAttachment(2, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL)
                        .withInputAttachment(3, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL)
                        .withInputAttachment(4, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL)
                        .withColorAttachment(6, VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL)
                        .withResolveAttachment(7, VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL);
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
            .addSubpassDependency(1, 2,
                                  VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT,
                                  VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT,
                                  VK_ACCESS_COLOR_ATTACHMENT_READ_BIT | VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT,
                                  VK_ACCESS_COLOR_ATTACHMENT_READ_BIT | VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT)
            .build();

    this->_descriptorPool = DescriptorPoolBuilder(this->_renderingDevice)
            .forType(VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER)
            .forType(VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER)
            .build();

    this->_objectDescriptorSetLayout = DescriptorSetLayoutBuilder(this->_renderingDevice)
            .withBinding(0, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, VK_SHADER_STAGE_FRAGMENT_BIT)
            .build();

    this->_textureSampler = this->_renderingDevice->createSampler();

    this->initSkyboxPipeline();
    this->initScenePipeline();
    this->initCompositionPipeline();
}

void SceneRenderpass::destroyRenderpass() {
    this->destroyCompositionPipeline();
    this->destroyScenePipeline();
    this->destroySkyboxPipeline();

    this->_renderingDevice->destroyDescriptorSetLayout(this->_objectDescriptorSetLayout);
    this->_renderingDevice->destroyDescriptorPool(this->_descriptorPool);
    this->_renderingDevice->destroySampler(this->_textureSampler);

    RenderpassBase::destroyRenderpass();
}

void SceneRenderpass::createFramebuffers() {
    VkExtent2D extent = this->_swapchain->getSwapchainExtent();
    VkSampleCountFlagBits samples = this->_renderingDevice->getPhysicalDevice()->getMsaaSamples();
    VkFormat colorFormat = this->_renderingDevice->getPhysicalDevice()->getColorFormat();

    this->_skyboxImage = this->_renderingObjectsFactory->createImageObject(extent.width, extent.height, 1, 0,
                                                                           colorFormat,
                                                                           VK_IMAGE_USAGE_TRANSIENT_ATTACHMENT_BIT |
                                                                           VK_IMAGE_USAGE_INPUT_ATTACHMENT_BIT |
                                                                           VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT,
                                                                           samples,
                                                                           VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);
    this->_skyboxImageView = this->_renderingObjectsFactory->createImageViewObject(this->_skyboxImage,
                                                                                   VK_IMAGE_VIEW_TYPE_2D,
                                                                                   VK_IMAGE_ASPECT_COLOR_BIT);

    this->_albedoImage = this->_renderingObjectsFactory->createImageObject(extent.width, extent.height, 1, 0,
                                                                           VK_FORMAT_R8G8B8A8_UNORM,
                                                                           VK_IMAGE_USAGE_TRANSIENT_ATTACHMENT_BIT |
                                                                           VK_IMAGE_USAGE_INPUT_ATTACHMENT_BIT |
                                                                           VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT,
                                                                           samples,
                                                                           VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);
    this->_albedoImageView = this->_renderingObjectsFactory->createImageViewObject(this->_albedoImage,
                                                                                   VK_IMAGE_VIEW_TYPE_2D,
                                                                                   VK_IMAGE_ASPECT_COLOR_BIT);

    this->_positionImage = this->_renderingObjectsFactory->createImageObject(extent.width, extent.height, 1, 0,
                                                                             VK_FORMAT_R16G16B16A16_SFLOAT,
                                                                             VK_IMAGE_USAGE_TRANSIENT_ATTACHMENT_BIT |
                                                                             VK_IMAGE_USAGE_INPUT_ATTACHMENT_BIT |
                                                                             VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT,
                                                                             samples,
                                                                             VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);
    this->_positionImageView = this->_renderingObjectsFactory->createImageViewObject(this->_positionImage,
                                                                                     VK_IMAGE_VIEW_TYPE_2D,
                                                                                     VK_IMAGE_ASPECT_COLOR_BIT);

    this->_normalImage = this->_renderingObjectsFactory->createImageObject(extent.width, extent.height, 1, 0,
                                                                           VK_FORMAT_R16G16B16A16_SFLOAT,
                                                                           VK_IMAGE_USAGE_TRANSIENT_ATTACHMENT_BIT |
                                                                           VK_IMAGE_USAGE_INPUT_ATTACHMENT_BIT |
                                                                           VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT,
                                                                           samples,
                                                                           VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);
    this->_normalImageView = this->_renderingObjectsFactory->createImageViewObject(this->_normalImage,
                                                                                   VK_IMAGE_VIEW_TYPE_2D,
                                                                                   VK_IMAGE_ASPECT_COLOR_BIT);

    this->_specularImage = this->_renderingObjectsFactory->createImageObject(extent.width, extent.height, 1, 0,
                                                                             VK_FORMAT_R8G8B8A8_UNORM,
                                                                             VK_IMAGE_USAGE_TRANSIENT_ATTACHMENT_BIT |
                                                                             VK_IMAGE_USAGE_INPUT_ATTACHMENT_BIT |
                                                                             VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT,
                                                                             samples,
                                                                             VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);
    this->_specularImageView = this->_renderingObjectsFactory->createImageViewObject(this->_specularImage,
                                                                                     VK_IMAGE_VIEW_TYPE_2D,
                                                                                     VK_IMAGE_ASPECT_COLOR_BIT);

    this->_depthImage = this->_renderingObjectsFactory->createImageObject(extent.width, extent.height, 1, 0,
                                                                          this->_renderingDevice->getPhysicalDevice()->getDepthFormat(),
                                                                          VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT,
                                                                          samples,
                                                                          VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);
    this->_depthImageView = this->_renderingObjectsFactory->createImageViewObject(this->_depthImage,
                                                                                  VK_IMAGE_VIEW_TYPE_2D,
                                                                                  VK_IMAGE_ASPECT_DEPTH_BIT);

    this->_compositionImage = this->_renderingObjectsFactory->createImageObject(extent.width, extent.height, 1, 0,
                                                                                colorFormat,
                                                                                VK_IMAGE_USAGE_TRANSIENT_ATTACHMENT_BIT |
                                                                                VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT,
                                                                                samples,
                                                                                VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);
    this->_compositionImageView = this->_renderingObjectsFactory->createImageViewObject(this->_compositionImage,
                                                                                        VK_IMAGE_VIEW_TYPE_2D,
                                                                                        VK_IMAGE_ASPECT_COLOR_BIT);

    this->_resultImage = this->_renderingObjectsFactory->createImageObject(extent.width, extent.height, 1, 0,
                                                                           colorFormat,
                                                                           VK_IMAGE_USAGE_TRANSIENT_ATTACHMENT_BIT |
                                                                           VK_IMAGE_USAGE_INPUT_ATTACHMENT_BIT |
                                                                           VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT,
                                                                           VK_SAMPLE_COUNT_1_BIT,
                                                                           VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);
    this->_resultImageView = this->_renderingObjectsFactory->createImageViewObject(this->_resultImage,
                                                                                   VK_IMAGE_VIEW_TYPE_2D,
                                                                                   VK_IMAGE_ASPECT_COLOR_BIT);

    this->_compositionGBufferDescriptorSet = this->_renderingObjectsFactory->createDescriptorSetObject(
            this->_descriptorPool, this->_compositionGBufferDescriptorSetLayout, MAX_INFLIGHT_FRAMES);

    std::array<VkImageView, 5> compositionImages = {
            this->_skyboxImageView->getHandle(),
            this->_albedoImageView->getHandle(),
            this->_positionImageView->getHandle(),
            this->_normalImageView->getHandle(),
            this->_specularImageView->getHandle()
    };

    for (uint32_t frameIdx = 0; frameIdx < MAX_INFLIGHT_FRAMES; frameIdx++) {
        for (uint32_t imageIdx = 0; imageIdx < compositionImages.size(); imageIdx++) {
            VkDescriptorImageInfo imageInfo = {
                    .sampler = VK_NULL_HANDLE,
                    .imageView = compositionImages[imageIdx],
                    .imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL
            };

            std::vector<VkWriteDescriptorSet> writes = {
                    VkWriteDescriptorSet{
                            .sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET,
                            .pNext = nullptr,
                            .dstSet = this->_compositionGBufferDescriptorSet->getDescriptorSet(frameIdx),
                            .dstBinding = imageIdx,
                            .dstArrayElement = 0,
                            .descriptorCount = 1,
                            .descriptorType = VK_DESCRIPTOR_TYPE_INPUT_ATTACHMENT,
                            .pImageInfo = &imageInfo,
                            .pBufferInfo = nullptr,
                            .pTexelBufferView = nullptr
                    }
            };

            this->_renderingDevice->updateDescriptorSets(writes);
        }
    }

    FramebufferBuilder builder = FramebufferBuilder(this->_renderingDevice, this->_renderpass)
            .withExtent(extent)
            .addAttachment(this->_skyboxImageView->getHandle())
            .addAttachment(this->_albedoImageView->getHandle())
            .addAttachment(this->_positionImageView->getHandle())
            .addAttachment(this->_normalImageView->getHandle())
            .addAttachment(this->_specularImageView->getHandle())
            .addAttachment(this->_depthImageView->getHandle())
            .addAttachment(this->_compositionImageView->getHandle())
            .addAttachment(this->_resultImageView->getHandle());

    uint32_t count = this->_swapchain->getImageCount();
    this->_framebuffers.resize(count);

    for (uint32_t idx = 0; idx < count; idx++) {
        this->_framebuffers[idx] = builder.build();
    }
}

void SceneRenderpass::destroyFramebuffers() {
    RenderpassBase::destroyFramebuffers();

    delete this->_compositionGBufferDescriptorSet;

    delete this->_compositionImageView;
    delete this->_compositionImage;
    delete this->_specularImageView;
    delete this->_specularImage;
    delete this->_normalImageView;
    delete this->_normalImage;
    delete this->_positionImageView;
    delete this->_positionImage;
    delete this->_albedoImageView;
    delete this->_albedoImage;
    delete this->_depthImageView;
    delete this->_depthImage;
    delete this->_skyboxImageView;
    delete this->_skyboxImage;

    delete this->_resultImageView;
    delete this->_resultImage;
}

void SceneRenderpass::addShadowRenderpass(RenderpassBase *renderpass) {
    this->_shadowRenderpasses.push_back(renderpass);
}
