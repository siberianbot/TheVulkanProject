#include "SceneRenderpass.hpp"

#include "src/Engine/Engine.hpp"
#include "src/Engine/EngineVars.hpp"
#include "src/Events/EventQueue.hpp"
#include "src/Objects/Camera.hpp"
#include "src/Objects/Light.hpp"
#include "src/Objects/Object.hpp"
#include "src/Scene/Scene.hpp"
#include "src/Scene/SceneManager.hpp"
#include "src/Objects/Skybox.hpp"
#include "src/Objects/Data/RenderingData.hpp"
#include "src/Types/Vertex.hpp"
#include "src/Rendering/PhysicalDevice.hpp"
#include "src/Rendering/RenderingDevice.hpp"
#include "src/Rendering/Swapchain.hpp"
#include "src/Rendering/Builders/RenderpassBuilder.hpp"
#include "src/Rendering/Builders/FramebufferBuilder.hpp"
#include "src/Rendering/Builders/PipelineBuilder.hpp"
#include "src/Rendering/Builders/AttachmentBuilder.hpp"
#include "src/Rendering/Builders/SubpassBuilder.hpp"
#include "src/Rendering/Builders/DescriptorPoolBuilder.hpp"
#include "src/Rendering/Builders/DescriptorSetLayoutBuilder.hpp"
#include "src/Rendering/Builders/PipelineLayoutBuilder.hpp"
#include "src/Rendering/Builders/SamplerBuilder.hpp"
#include "src/Rendering/Objects/BufferObject.hpp"
#include "src/Rendering/Objects/ImageObject.hpp"
#include "src/Rendering/Objects/ImageViewObject.hpp"
#include "src/Rendering/Objects/DescriptorSetObject.hpp"
#include "src/Resources/ResourceManager.hpp"
#include "src/Resources/CubeImageResource.hpp"
#include "src/Resources/ImageResource.hpp"
#include "src/Resources/MeshResource.hpp"
#include "src/Resources/ShaderResource.hpp"

std::shared_ptr<RenderingData> SceneRenderpass::getRenderData(Object *object) {
    auto it = object->data().find(RENDERING_DATA_TYPE);

    std::shared_ptr<RenderingData> renderData;

    if (it != object->data().end()) {
        renderData = std::dynamic_pointer_cast<RenderingData>(it->second);
    } else {
        renderData = std::make_shared<RenderingData>();
        renderData->descriptorSet = DescriptorSetObject::create(this->_renderingDevice.get(), MAX_INFLIGHT_FRAMES,
                                                                this->_descriptorPool,
                                                                this->_objectDescriptorSetLayout);

        object->data()[RENDERING_DATA_TYPE] = renderData;
    }

    ImageObject *albedoTexture = object->albedoTexture() == nullptr
                                 ? this->_engine->resourceManager()->loadDefaultImage()->image()
                                 : object->albedoTexture()->image();

    if (renderData->albedoTextureView == nullptr || albedoTexture != renderData->albedoTextureView->getImage()) {
        renderData->albedoTextureView = getImageView(albedoTexture);

        updateDescriptorSetWithImage(renderData->descriptorSet, renderData->albedoTextureView, 0);
    }

    ImageObject *specTexture = object->specTexture() == nullptr
                               ? this->_engine->resourceManager()->loadDefaultImage()->image()
                               : object->specTexture()->image();

    if (renderData->specTextureView == nullptr || specTexture != renderData->specTextureView->getImage()) {
        renderData->specTextureView = getImageView(specTexture);

        updateDescriptorSetWithImage(renderData->descriptorSet, renderData->specTextureView, 1);
    }

    return renderData;
}

ImageViewObject *SceneRenderpass::getImageView(ImageObject *image) {
    auto it = this->_imageViews.find(image);

    if (it != this->_imageViews.end()) {
        return it->second;
    }

    ImageViewObject *imageView = ImageViewObject::create(this->_renderingDevice.get(), image, VK_IMAGE_VIEW_TYPE_2D,
                                                         VK_IMAGE_ASPECT_COLOR_BIT);

    this->_imageViews[image] = imageView;

    return imageView;
}

void SceneRenderpass::updateDescriptorSetWithImage(DescriptorSetObject *descriptorSetObject,
                                                   ImageViewObject *imageViewObject, uint32_t binding) {
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
                .dstBinding = binding,
                .dstArrayElement = 0,
                .descriptorCount = 1,
                .descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER,
                .pImageInfo = &imageInfo,
                .pBufferInfo = nullptr,
                .pTexelBufferView = nullptr
        });
    }

    this->_renderingDevice.get()->updateDescriptorSets(writes);
}

void SceneRenderpass::initSkyboxPipeline() {
    this->_skyboxPipelineLayout = PipelineLayoutBuilder(this->_renderingDevice.get())
            .withDescriptorSetLayout(this->_objectDescriptorSetLayout)
            .withPushConstant(VK_SHADER_STAGE_VERTEX_BIT, 0, sizeof(MeshConstants))
            .build();

    ShaderResource *vertexShader = this->_engine->resourceManager()->loadShader("default_vert");
    ShaderResource *fragmentShader = this->_engine->resourceManager()->loadShader("skybox_frag");

    this->_skyboxPipeline = PipelineBuilder(this->_renderingDevice.get(), this->_renderpass, this->_skyboxPipelineLayout)
            .addVertexShader(vertexShader->shader())
            .addFragmentShader(fragmentShader->shader())
            .addBinding(0, sizeof(Vertex), VK_VERTEX_INPUT_RATE_VERTEX)
            .addAttribute(0, 0, offsetof(Vertex, pos), VK_FORMAT_R32G32B32_SFLOAT)
            .addAttribute(0, 1, offsetof(Vertex, normal), VK_FORMAT_R32G32B32_SFLOAT)
            .addAttribute(0, 2, offsetof(Vertex, color), VK_FORMAT_R32G32B32_SFLOAT)
            .addAttribute(0, 3, offsetof(Vertex, uv), VK_FORMAT_R32G32_SFLOAT)
            .withRasterizationSamples(this->_renderingDevice.get()->getPhysicalDevice()->getMsaaSamples())
            .forSubpass(0)
            .build();

    vertexShader->unload();
    fragmentShader->unload();

    this->_skyboxDescriptorSet = DescriptorSetObject::create(this->_renderingDevice.get(), MAX_INFLIGHT_FRAMES,
                                                             this->_descriptorPool, this->_objectDescriptorSetLayout);

    Scene *currentScene = this->_engine->sceneManager()->currentScene();
    if (currentScene != nullptr) {
        this->_skyboxTextureView = ImageViewObject::create(this->_renderingDevice.get(),
                                                           currentScene->skybox()->texture()->image(),
                                                           VK_IMAGE_VIEW_TYPE_CUBE,
                                                           VK_IMAGE_ASPECT_COLOR_BIT);

        updateDescriptorSetWithImage(this->_skyboxDescriptorSet, this->_skyboxTextureView, 0);
    }
}

void SceneRenderpass::destroySkyboxPipeline() {
    delete this->_skyboxDescriptorSet;

    if (this->_skyboxTextureView != nullptr) {
        delete this->_skyboxTextureView;
        this->_skyboxTextureView = nullptr;
    }

    this->_renderingDevice.get()->destroyPipeline(this->_skyboxPipeline);
    this->_renderingDevice.get()->destroyPipelineLayout(this->_skyboxPipelineLayout);
}

void SceneRenderpass::initScenePipeline() {
    this->_scenePipelineLayout = PipelineLayoutBuilder(this->_renderingDevice.get())
            .withDescriptorSetLayout(this->_objectDescriptorSetLayout)
            .withPushConstant(VK_SHADER_STAGE_VERTEX_BIT, 0, sizeof(MeshConstants))
            .build();

    ShaderResource *vertexShader = this->_engine->resourceManager()->loadShader("default_vert");
    ShaderResource *fragmentShader = this->_engine->resourceManager()->loadShader("default_frag");

    this->_scenePipeline = PipelineBuilder(this->_renderingDevice.get(), this->_renderpass, this->_scenePipelineLayout)
            .addVertexShader(vertexShader->shader())
            .addFragmentShader(fragmentShader->shader())
            .addBinding(0, sizeof(Vertex), VK_VERTEX_INPUT_RATE_VERTEX)
            .addAttribute(0, 0, offsetof(Vertex, pos), VK_FORMAT_R32G32B32_SFLOAT)
            .addAttribute(0, 1, offsetof(Vertex, normal), VK_FORMAT_R32G32B32_SFLOAT)
            .addAttribute(0, 2, offsetof(Vertex, color), VK_FORMAT_R32G32B32_SFLOAT)
            .addAttribute(0, 3, offsetof(Vertex, uv), VK_FORMAT_R32G32_SFLOAT)
            .withRasterizationSamples(this->_renderingDevice.get()->getPhysicalDevice()->getMsaaSamples())
            .withColorBlendAttachmentCount(4)
            .forSubpass(1)
            .build();

    vertexShader->unload();
    fragmentShader->unload();
}

void SceneRenderpass::destroyScenePipeline() {
    for (const auto &item: this->_imageViews) {
        delete item.second;
    }

    this->_imageViews.clear();

    this->_renderingDevice.get()->destroyPipeline(this->_scenePipeline);
    this->_renderingDevice.get()->destroyPipelineLayout(this->_scenePipelineLayout);
}

void SceneRenderpass::initCompositionPipeline() {
    this->_compositionGBufferDescriptorSetLayout = DescriptorSetLayoutBuilder(this->_renderingDevice.get())
            .withBinding(0, VK_DESCRIPTOR_TYPE_INPUT_ATTACHMENT, VK_SHADER_STAGE_FRAGMENT_BIT)
            .withBinding(1, VK_DESCRIPTOR_TYPE_INPUT_ATTACHMENT, VK_SHADER_STAGE_FRAGMENT_BIT)
            .withBinding(2, VK_DESCRIPTOR_TYPE_INPUT_ATTACHMENT, VK_SHADER_STAGE_FRAGMENT_BIT)
            .withBinding(3, VK_DESCRIPTOR_TYPE_INPUT_ATTACHMENT, VK_SHADER_STAGE_FRAGMENT_BIT)
            .withBinding(4, VK_DESCRIPTOR_TYPE_INPUT_ATTACHMENT, VK_SHADER_STAGE_FRAGMENT_BIT)
            .build();

    this->_compositionSceneDataDescriptorSetLayout = DescriptorSetLayoutBuilder(this->_renderingDevice.get())
            .withBinding(0, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, VK_SHADER_STAGE_FRAGMENT_BIT)
            .withBinding(1, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, VK_SHADER_STAGE_FRAGMENT_BIT, MAX_NUM_LIGHTS)
            .build();

    this->_compositionSceneDataBuffer = BufferObject::create(this->_renderingDevice.get(), sizeof(SceneData),
                                                             VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT,
                                                             VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT |
                                                             VK_MEMORY_PROPERTY_HOST_COHERENT_BIT);

    this->_compositionSceneData = reinterpret_cast<SceneData *>(this->_compositionSceneDataBuffer->map());

    this->_compositionSceneDataDescriptorSet = DescriptorSetObject::create(this->_renderingDevice.get(), MAX_INFLIGHT_FRAMES,
                                                                           this->_descriptorPool,
                                                                           this->_compositionSceneDataDescriptorSetLayout);

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

        this->_renderingDevice.get()->updateDescriptorSets(writes);
    }

    this->_compositionPipelineLayout = PipelineLayoutBuilder(this->_renderingDevice.get())
            .withDescriptorSetLayout(this->_compositionGBufferDescriptorSetLayout)
            .withDescriptorSetLayout(this->_compositionSceneDataDescriptorSetLayout)
            .build();

    ShaderResource *vertexShader = this->_engine->resourceManager()->loadShader("composition_vert");
    ShaderResource *fragmentShader = this->_engine->resourceManager()->loadShader("scene_composition_frag");

    this->_compositionPipeline = PipelineBuilder(this->_renderingDevice.get(), this->_renderpass,
                                                 this->_compositionPipelineLayout)
            .addVertexShader(vertexShader->shader())
            .addFragmentShader(fragmentShader->shader())
            .withRasterizationSamples(this->_renderingDevice.get()->getPhysicalDevice()->getMsaaSamples())
            .withCullMode(VK_CULL_MODE_FRONT_BIT)
            .forSubpass(2)
            .build();

    vertexShader->unload();
    fragmentShader->unload();
}

void SceneRenderpass::destroyCompositionPipeline() {
    this->_renderingDevice.get()->destroyPipeline(this->_compositionPipeline);
    this->_renderingDevice.get()->destroyPipelineLayout(this->_compositionPipelineLayout);
    this->_renderingDevice.get()->destroyDescriptorSetLayout(this->_compositionSceneDataDescriptorSetLayout);
    this->_renderingDevice.get()->destroyDescriptorSetLayout(this->_compositionGBufferDescriptorSetLayout);
    delete this->_compositionSceneDataDescriptorSet;
    delete this->_compositionSceneDataBuffer;
}

SceneRenderpass::SceneRenderpass(const std::shared_ptr<RenderingDevice> &renderingDevice, Swapchain *swapchain, Engine *engine)
        : RenderpassBase(renderingDevice),
          _engine(engine),
          _swapchain(swapchain) {
    this->_engine->eventQueue()->addHandler([this](const Event &event) {
        if (event.type != SCENE_TRANSITION_EVENT) {
            return;
        }

        if (this->_skyboxTextureView != nullptr) {
            delete this->_skyboxTextureView;
        }

        this->_skyboxTextureView = ImageViewObject::create(this->_renderingDevice.get(),
                                                           event.scene->skybox()->texture()->image(),
                                                           VK_IMAGE_VIEW_TYPE_CUBE,
                                                           VK_IMAGE_ASPECT_COLOR_BIT);

        updateDescriptorSetWithImage(this->_skyboxDescriptorSet, this->_skyboxTextureView, 0);
    });
}

void SceneRenderpass::recordCommands(VkCommandBuffer commandBuffer, VkRect2D renderArea,
                                     uint32_t frameIdx, uint32_t imageIdx) {
    Scene *currentScene = this->_engine->sceneManager()->currentScene();

    // PREPARE SCENE DATA
    {
        std::vector<LightData> lightData;
        if (currentScene != nullptr) {
            glm::vec3 cameraPosition = currentScene->camera()->position();

            for (uint32_t idx = 0; idx < currentScene->lights().size(); idx++) {
                Light *light = currentScene->lights()[idx];

                if (!light->enabled() || glm::distance(cameraPosition, light->position()) > 100) {
                    continue;
                }

                glm::mat4 projection = light->getProjectionMatrix();
                if (light->kind() == POINT_LIGHT) {
                    for (glm::vec3 forward: POINT_LIGHT_DIRECTIONS) {
                        glm::mat4 view = light->getViewMatrix(forward);
                        lightData.push_back(LightData{
                                projection * view,
                                light->position(),
                                light->color(),
                                light->radius(),
                                (int) light->kind()
                        });
                    }
                } else {
                    glm::mat4 view = light->getViewMatrix();
                    lightData.push_back(LightData{
                            projection * view,
                            light->position(),
                            light->color(),
                            light->radius(),
                            (int) light->kind()
                    });
                }
            }

            this->_compositionSceneData->cameraPosition = cameraPosition;
        }

        this->_compositionSceneData->numLights = std::min(MAX_NUM_LIGHTS, (int) lightData.size());
        for (int idx = 0; idx < this->_compositionSceneData->numLights; idx++) {
            this->_compositionSceneData->lights[idx] = lightData[idx];
        }
    }

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
    if (this->_engine->engineVars()->getOrDefault(RENDERER_SKYBOX_ENABLED_VAR, true)->boolValue &&
        currentScene != nullptr) {
        glm::mat4 projection = currentScene->camera()->getProjectionMatrix(renderArea.extent.width,
                                                                           renderArea.extent.height);
        projection[1][1] *= -1;

        vkCmdBindPipeline(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, this->_skyboxPipeline);

        MeshConstants constants = {
                .matrix = projection * currentScene->camera()->getViewMatrix(true) * glm::mat4(1),
                .model = glm::mat4(1)
        };
        vkCmdPushConstants(commandBuffer, this->_skyboxPipelineLayout, VK_SHADER_STAGE_VERTEX_BIT, 0,
                           sizeof(MeshConstants), &constants);

        VkDescriptorSet descriptorSet = this->_skyboxDescriptorSet->getDescriptorSet(frameIdx);
        vkCmdBindDescriptorSets(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, this->_skyboxPipelineLayout,
                                0, 1, &descriptorSet, 0, nullptr);

        VkBuffer vertexBuffer = currentScene->skybox()->mesh()->vertexBuffer()->getHandle();
        vkCmdBindVertexBuffers(commandBuffer, 0, 1, &vertexBuffer, &offset);
        vkCmdBindIndexBuffer(commandBuffer, currentScene->skybox()->mesh()->indexBuffer()->getHandle(), 0,
                             VK_INDEX_TYPE_UINT32);

        vkCmdDrawIndexed(commandBuffer, currentScene->skybox()->mesh()->count(), 1, 0, 0, 0);
    }

    vkCmdNextSubpass(commandBuffer, VK_SUBPASS_CONTENTS_INLINE);

    // SCENE
    if (currentScene != nullptr) {
        glm::mat4 projection = currentScene->camera()->getProjectionMatrix(renderArea.extent.width,
                                                                           renderArea.extent.height);
        projection[1][1] *= -1;

        vkCmdBindPipeline(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, this->_scenePipeline);

        glm::mat4 view = currentScene->camera()->getViewMatrix(false);

        uint32_t idx = 0;
        for (Object *object: currentScene->objects()) {
            if (object->mesh() == nullptr) {
                continue;
            }

            glm::mat4 model = object->getModelMatrix(false);
            glm::mat4 rot = object->getModelMatrix(true);
            MeshConstants constants = {
                    .matrix = projection * view * model,
                    .model = model,
                    .modelRotation = rot
            };
            vkCmdPushConstants(commandBuffer, this->_scenePipelineLayout, VK_SHADER_STAGE_VERTEX_BIT, 0,
                               sizeof(MeshConstants), &constants);

            VkBuffer vertexBuffer = object->mesh()->vertexBuffer()->getHandle();
            vkCmdBindVertexBuffers(commandBuffer, 0, 1, &vertexBuffer, &offset);
            vkCmdBindIndexBuffer(commandBuffer, object->mesh()->indexBuffer()->getHandle(), 0, VK_INDEX_TYPE_UINT32);

            VkDescriptorSet descriptor = getRenderData(object)->descriptorSet->getDescriptorSet(frameIdx);
            vkCmdBindDescriptorSets(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, this->_scenePipelineLayout,
                                    0, 1, &descriptor, 0, nullptr);

            vkCmdDrawIndexed(commandBuffer, object->mesh()->count(), 1, 0, 0, idx++);
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
    VkFormat format = this->_renderingDevice.get()->getPhysicalDevice()->getColorFormat();
    VkSampleCountFlagBits samples = this->_renderingDevice.get()->getPhysicalDevice()->getMsaaSamples();
    this->_renderpass = RenderpassBuilder(this->_renderingDevice.get())
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
                        .withFormat(this->_renderingDevice.get()->getPhysicalDevice()->getDepthFormat())
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

    this->_descriptorPool = DescriptorPoolBuilder(this->_renderingDevice.get())
            .forType(VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER)
            .forType(VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER)
            .build();

    this->_objectDescriptorSetLayout = DescriptorSetLayoutBuilder(this->_renderingDevice.get())
            .withBinding(0, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, VK_SHADER_STAGE_FRAGMENT_BIT)
            .withBinding(1, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, VK_SHADER_STAGE_FRAGMENT_BIT)
            .build();

    this->_textureSampler = SamplerBuilder(this->_renderingDevice)
            .withAddressMode(VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_BORDER)
            .withBorderColor(VK_BORDER_COLOR_FLOAT_OPAQUE_WHITE)
            .enableAnisotropy(this->_renderingDevice.get()->getPhysicalDevice()->getMaxSamplerAnisotropy())
            .build();

    this->initSkyboxPipeline();
    this->initScenePipeline();
    this->initCompositionPipeline();
}

void SceneRenderpass::destroyRenderpass() {
    this->destroyCompositionPipeline();
    this->destroyScenePipeline();
    this->destroySkyboxPipeline();

    if (this->_engine->sceneManager()->currentScene() != nullptr) {
        for (Object *object: this->_engine->sceneManager()->currentScene()->objects()) {
            object->data()[RENDERING_DATA_TYPE] = nullptr;
        }
    }

    this->_renderingDevice.get()->destroyDescriptorSetLayout(this->_objectDescriptorSetLayout);
    this->_renderingDevice.get()->destroyDescriptorPool(this->_descriptorPool);
    this->_renderingDevice.get()->destroySampler(this->_textureSampler);

    RenderpassBase::destroyRenderpass();
}

void SceneRenderpass::createFramebuffers() {
    VkExtent2D extent = this->_swapchain->getSwapchainExtent();
    VkSampleCountFlagBits samples = this->_renderingDevice.get()->getPhysicalDevice()->getMsaaSamples();
    VkFormat colorFormat = this->_renderingDevice.get()->getPhysicalDevice()->getColorFormat();

    this->_skyboxImage = ImageObject::create(this->_renderingDevice.get(), extent.width, extent.height, 1, 0,
                                             colorFormat,
                                             VK_IMAGE_USAGE_TRANSIENT_ATTACHMENT_BIT |
                                             VK_IMAGE_USAGE_INPUT_ATTACHMENT_BIT |
                                             VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT,
                                             samples,
                                             VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);
    this->_skyboxImageView = ImageViewObject::create(this->_renderingDevice.get(), this->_skyboxImage,
                                                     VK_IMAGE_VIEW_TYPE_2D,
                                                     VK_IMAGE_ASPECT_COLOR_BIT);

    this->_albedoImage = ImageObject::create(this->_renderingDevice.get(), extent.width, extent.height, 1, 0,
                                             VK_FORMAT_R8G8B8A8_UNORM,
                                             VK_IMAGE_USAGE_TRANSIENT_ATTACHMENT_BIT |
                                             VK_IMAGE_USAGE_INPUT_ATTACHMENT_BIT |
                                             VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT,
                                             samples,
                                             VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);
    this->_albedoImageView = ImageViewObject::create(this->_renderingDevice.get(), this->_albedoImage,
                                                     VK_IMAGE_VIEW_TYPE_2D,
                                                     VK_IMAGE_ASPECT_COLOR_BIT);

    this->_positionImage = ImageObject::create(this->_renderingDevice.get(), extent.width, extent.height, 1, 0,
                                               VK_FORMAT_R16G16B16A16_SFLOAT,
                                               VK_IMAGE_USAGE_TRANSIENT_ATTACHMENT_BIT |
                                               VK_IMAGE_USAGE_INPUT_ATTACHMENT_BIT |
                                               VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT,
                                               samples,
                                               VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);
    this->_positionImageView = ImageViewObject::create(this->_renderingDevice.get(), this->_positionImage,
                                                       VK_IMAGE_VIEW_TYPE_2D,
                                                       VK_IMAGE_ASPECT_COLOR_BIT);

    this->_normalImage = ImageObject::create(this->_renderingDevice.get(), extent.width, extent.height, 1, 0,
                                             VK_FORMAT_R16G16B16A16_SFLOAT,
                                             VK_IMAGE_USAGE_TRANSIENT_ATTACHMENT_BIT |
                                             VK_IMAGE_USAGE_INPUT_ATTACHMENT_BIT |
                                             VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT,
                                             samples,
                                             VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);
    this->_normalImageView = ImageViewObject::create(this->_renderingDevice.get(), this->_normalImage,
                                                     VK_IMAGE_VIEW_TYPE_2D,
                                                     VK_IMAGE_ASPECT_COLOR_BIT);

    this->_specularImage = ImageObject::create(this->_renderingDevice.get(), extent.width, extent.height, 1, 0,
                                               VK_FORMAT_R8G8B8A8_UNORM,
                                               VK_IMAGE_USAGE_TRANSIENT_ATTACHMENT_BIT |
                                               VK_IMAGE_USAGE_INPUT_ATTACHMENT_BIT |
                                               VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT,
                                               samples,
                                               VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);
    this->_specularImageView = ImageViewObject::create(this->_renderingDevice.get(), this->_specularImage,
                                                       VK_IMAGE_VIEW_TYPE_2D,
                                                       VK_IMAGE_ASPECT_COLOR_BIT);

    this->_depthImage = ImageObject::create(this->_renderingDevice.get(), extent.width, extent.height, 1, 0,
                                            this->_renderingDevice->getPhysicalDevice()->getDepthFormat(),
                                            VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT,
                                            samples,
                                            VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);
    this->_depthImageView = ImageViewObject::create(this->_renderingDevice.get(), this->_depthImage,
                                                    VK_IMAGE_VIEW_TYPE_2D,
                                                    VK_IMAGE_ASPECT_DEPTH_BIT);

    this->_compositionImage = ImageObject::create(this->_renderingDevice.get(), extent.width, extent.height, 1, 0,
                                                  colorFormat,
                                                  VK_IMAGE_USAGE_TRANSIENT_ATTACHMENT_BIT |
                                                  VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT,
                                                  samples,
                                                  VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);
    this->_compositionImageView = ImageViewObject::create(this->_renderingDevice.get(), this->_compositionImage,
                                                          VK_IMAGE_VIEW_TYPE_2D,
                                                          VK_IMAGE_ASPECT_COLOR_BIT);

    this->_resultImage = ImageObject::create(this->_renderingDevice.get(), extent.width, extent.height, 1, 0,
                                             colorFormat,
                                             VK_IMAGE_USAGE_TRANSIENT_ATTACHMENT_BIT |
                                             VK_IMAGE_USAGE_INPUT_ATTACHMENT_BIT |
                                             VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT,
                                             VK_SAMPLE_COUNT_1_BIT,
                                             VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);
    this->_resultImageView = ImageViewObject::create(this->_renderingDevice.get(), this->_resultImage,
                                                     VK_IMAGE_VIEW_TYPE_2D,
                                                     VK_IMAGE_ASPECT_COLOR_BIT);

    this->_compositionGBufferDescriptorSet = DescriptorSetObject::create(this->_renderingDevice.get(), MAX_INFLIGHT_FRAMES,
                                                                         this->_descriptorPool,
                                                                         this->_compositionGBufferDescriptorSetLayout);

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

    FramebufferBuilder builder = FramebufferBuilder(this->_renderingDevice.get(), this->_renderpass)
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
