#include "ShadowRenderpass.hpp"

#include "src/Engine.hpp"
#include "src/Types/Vertex.hpp"
#include "src/Rendering/PhysicalDevice.hpp"
#include "src/Rendering/RenderingDevice.hpp"
#include "src/Rendering/Builders/RenderpassBuilder.hpp"
#include "src/Rendering/Builders/AttachmentBuilder.hpp"
#include "src/Rendering/Builders/SubpassBuilder.hpp"
#include "src/Rendering/Builders/PipelineLayoutBuilder.hpp"
#include "src/Rendering/Builders/PipelineBuilder.hpp"
#include "src/Rendering/Builders/FramebufferBuilder.hpp"
#include "src/Rendering/Objects/BufferObject.hpp"
#include "src/Rendering/Objects/ImageObject.hpp"
#include "src/Rendering/Objects/ImageViewObject.hpp"
#include "src/Resources/ResourceManager.hpp"
#include "src/Resources/MeshResource.hpp"
#include "src/Resources/ShaderResource.hpp"
#include "src/Objects/Camera.hpp"
#include "src/Objects/Light.hpp"
#include "src/Objects/Object.hpp"
#include "src/Scene/Scene.hpp"
#include "src/Scene/SceneManager.hpp"

static constexpr const uint32_t SIZE = 1024;

ShadowRenderpass::ShadowRenderpass(RenderingDevice *renderingDevice, Engine *engine)
        : RenderpassBase(renderingDevice),
          _engine(engine) {
    //
}

void ShadowRenderpass::recordCommands(VkCommandBuffer commandBuffer, VkRect2D renderArea, uint32_t frameIdx,
                                      uint32_t imageIdx) {
    Scene *currentScene = this->_engine->sceneManager()->currentScene();

    std::vector<LightData> lightData;
    if (currentScene != nullptr) {
        for (uint32_t idx = 0; idx < currentScene->lights().size(); idx++) {
            Light *light = currentScene->lights()[idx];

            if (!light->enabled() || glm::distance(currentScene->camera()->position(), light->position()) > 100) {
                continue;
            }

            glm::mat4 projection = light->getProjectionMatrix();
            if (light->kind() == POINT_LIGHT) {
                for (glm::vec3 forward: POINT_LIGHT_DIRECTIONS) {
                    glm::mat4 view = light->getViewMatrix(forward);
                    lightData.push_back(LightData{projection, view});
                }
            } else {
                glm::mat4 view = light->getViewMatrix();
                lightData.push_back(LightData{projection, view});
            }
        }
    }

    const std::array<VkClearValue, 1> clearValues = {
            VkClearValue{.depthStencil = {1, 0}},
    };

    VkRenderPassBeginInfo renderPassBeginInfo = {
            .sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO,
            .pNext = nullptr,
            .renderPass = this->_renderpass,
            .framebuffer = VK_NULL_HANDLE,
            .renderArea = {
                    .offset = {0, 0},
                    .extent = {SIZE, SIZE}
            },
            .clearValueCount = static_cast<uint32_t>(clearValues.size()),
            .pClearValues = clearValues.data()
    };

    const VkViewport viewport = {
            .x = 0,
            .y = 0,
            .width = (float) SIZE,
            .height = (float) SIZE,
            .minDepth = 0.0f,
            .maxDepth = 1.0f
    };

    const VkRect2D scissor = {
            .offset = {0, 0},
            .extent = {SIZE, SIZE}
    };

    const VkDeviceSize offset = 0;

    uint32_t count = std::min(MAX_NUM_LIGHTS, (int) lightData.size());

    for (uint32_t lightIdx = 0; lightIdx < MAX_NUM_LIGHTS; lightIdx++) {
        renderPassBeginInfo.framebuffer = this->_framebuffers[lightIdx];
        vkCmdBeginRenderPass(commandBuffer, &renderPassBeginInfo, VK_SUBPASS_CONTENTS_INLINE);

        vkCmdSetViewport(commandBuffer, 0, 1, &viewport);
        vkCmdSetScissor(commandBuffer, 0, 1, &scissor);
        vkCmdSetDepthBias(commandBuffer, 1.25f, 0.0f, 1.75f);

        if (lightIdx >= count || currentScene == nullptr) {
            vkCmdEndRenderPass(commandBuffer);

            continue;
        }

        LightData datum = lightData[lightIdx];

        vkCmdBindPipeline(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, this->_pipeline);

        uint32_t idx = 0;
        for (Object *object: currentScene->objects()) {
            if (object->mesh() == nullptr) {
                continue;
            }

            glm::mat4 model = object->getModelMatrix(false);
            MeshConstants constants = {
                    .matrix = datum.projection * datum.view * model,
            };
            vkCmdPushConstants(commandBuffer, this->_pipelineLayout, VK_SHADER_STAGE_VERTEX_BIT, 0,
                               sizeof(MeshConstants), &constants);

            VkBuffer vertexBuffer = object->mesh()->vertexBuffer()->getHandle();
            vkCmdBindVertexBuffers(commandBuffer, 0, 1, &vertexBuffer, &offset);
            vkCmdBindIndexBuffer(commandBuffer, object->mesh()->indexBuffer()->getHandle(), 0, VK_INDEX_TYPE_UINT32);

            vkCmdDrawIndexed(commandBuffer, object->mesh()->count(), 1, 0, 0, idx++);
        }

        vkCmdEndRenderPass(commandBuffer);
    }
}

void ShadowRenderpass::initRenderpass() {
    this->_renderpass = RenderpassBuilder(this->_renderingDevice)
            .addAttachment([&](AttachmentBuilder &builder) {
                builder
                        .clear()
                        .withFormat(this->_renderingDevice->getPhysicalDevice()->getDepthFormat())
                        .withFinalLayout(VK_IMAGE_LAYOUT_DEPTH_STENCIL_READ_ONLY_OPTIMAL);
            })
            .addSubpass([](SubpassBuilder &builder) {
                builder.withDepthAttachment(0, VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL);
            })
            .addSubpassDependency(VK_SUBPASS_EXTERNAL, 0,
                                  VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT,
                                  VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT,
                                  0,
                                  VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT)
            .addSubpassDependency(0, VK_SUBPASS_EXTERNAL,
                                  VK_PIPELINE_STAGE_LATE_FRAGMENT_TESTS_BIT,
                                  VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT,
                                  0,
                                  VK_ACCESS_SHADER_READ_BIT)
            .build();

    this->_pipelineLayout = PipelineLayoutBuilder(this->_renderingDevice)
            .withPushConstant(VK_SHADER_STAGE_VERTEX_BIT, 0, sizeof(MeshConstants))
            .build();

    ShaderResource *vertexShader = this->_engine->resourceManager()->loadShader("shadow_vert");
    ShaderResource *fragmentShader = this->_engine->resourceManager()->loadShader("shadow_frag");

    this->_pipeline = PipelineBuilder(this->_renderingDevice, this->_renderpass, this->_pipelineLayout)
            .addVertexShader(vertexShader->shader())
            .addFragmentShader(fragmentShader->shader())
            .addBinding(0, sizeof(Vertex), VK_VERTEX_INPUT_RATE_VERTEX)
            .addAttribute(0, 0, offsetof(Vertex, pos), VK_FORMAT_R32G32B32_SFLOAT)
            .withCullMode(VK_CULL_MODE_NONE)
            .withDepthBias()
            .build();

    vertexShader->unload();
    fragmentShader->unload();

    for (uint32_t idx = 0; idx < MAX_NUM_LIGHTS; idx++) {
        this->_depthImages[idx] = ImageObject::create(this->_renderingDevice, SIZE, SIZE, 1, 0,
                                                      this->_renderingDevice->getPhysicalDevice()->getDepthFormat(),
                                                      VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT |
                                                      VK_IMAGE_USAGE_SAMPLED_BIT,
                                                      VK_SAMPLE_COUNT_1_BIT,
                                                      VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);

        this->_depthImageViews[idx] = ImageViewObject::create(this->_renderingDevice, this->_depthImages[idx],
                                                              VK_IMAGE_VIEW_TYPE_2D, VK_IMAGE_ASPECT_DEPTH_BIT);
    }
}

void ShadowRenderpass::destroyRenderpass() {
    for (uint32_t idx = 0; idx < MAX_NUM_LIGHTS; idx++) {
        delete this->_depthImageViews[idx];
        delete this->_depthImages[idx];
    }

    this->_renderingDevice->destroyPipeline(this->_pipeline);
    this->_renderingDevice->destroyPipelineLayout(this->_pipelineLayout);

    RenderpassBase::destroyRenderpass();
}

void ShadowRenderpass::createFramebuffers() {
    VkExtent2D extent = {SIZE, SIZE};

    this->_framebuffers.resize(MAX_NUM_LIGHTS);

    for (uint32_t idx = 0; idx < MAX_NUM_LIGHTS; idx++) {
        this->_framebuffers[idx] = FramebufferBuilder(this->_renderingDevice, this->_renderpass)
                .withExtent(extent)
                .addAttachment(this->_depthImageViews[idx]->getHandle())
                .build();
    }
}

void ShadowRenderpass::destroyFramebuffers() {
    RenderpassBase::destroyFramebuffers();
}
