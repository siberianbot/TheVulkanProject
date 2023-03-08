#include "ShadowRenderpass.hpp"

#include "src/Engine.hpp"
#include "src/Resources/Vertex.hpp"
#include "src/Rendering/PhysicalDevice.hpp"
#include "src/Rendering/RenderingDevice.hpp"
#include "src/Rendering/RenderingObjectsFactory.hpp"
#include "src/Rendering/Builders/RenderpassBuilder.hpp"
#include "src/Rendering/Builders/AttachmentBuilder.hpp"
#include "src/Rendering/Builders/SubpassBuilder.hpp"
#include "src/Rendering/Builders/PipelineLayoutBuilder.hpp"
#include "src/Rendering/Builders/PipelineBuilder.hpp"
#include "src/Rendering/Builders/FramebufferBuilder.hpp"
#include "src/Rendering/Objects/BufferObject.hpp"
#include "src/Rendering/Objects/ImageObject.hpp"
#include "src/Rendering/Objects/ImageViewObject.hpp"
#include "src/Scene/Light.hpp"
#include "src/Scene/Object.hpp"
#include "src/Scene/Scene.hpp"

static constexpr const uint32_t SIZE = 1024;

ShadowRenderpass::ShadowRenderpass(RenderingDevice *renderingDevice, Engine *engine,
                                   RenderingObjectsFactory *renderingObjectsFactory)
        : RenderpassBase(renderingDevice),
          _engine(engine),
          _renderingObjectsFactory(renderingObjectsFactory) {
    //
}

void ShadowRenderpass::recordCommands(VkCommandBuffer commandBuffer, VkRect2D renderArea, uint32_t frameIdx,
                                      uint32_t imageIdx) {
    glm::vec3 cameraPosition = this->_engine->camera().position();

    std::vector<Light *> lights(this->_engine->scene()->lights().size());
    std::copy_if(this->_engine->scene()->lights().begin(), this->_engine->scene()->lights().end(), lights.begin(),
                 [&](Light *l) {
                     return glm::distance(cameraPosition, l->position()) < 100;
                 });

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

    uint32_t count = std::min(MAX_NUM_LIGHTS, (int) lights.size());

    for (uint32_t lightIdx = 0; lightIdx < count; lightIdx++) {
        Light *light = lights[lightIdx];
        glm::mat4 projection = light->getProjectionMatrix();
        glm::mat4 view = light->getViewMatrix();

        renderPassBeginInfo.framebuffer = this->_framebuffers[lightIdx];
        vkCmdBeginRenderPass(commandBuffer, &renderPassBeginInfo, VK_SUBPASS_CONTENTS_INLINE);

        vkCmdSetViewport(commandBuffer, 0, 1, &viewport);
        vkCmdSetScissor(commandBuffer, 0, 1, &scissor);
        vkCmdSetDepthBias(commandBuffer, 1.25f, 0.0f, 1.75f);

        vkCmdBindPipeline(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, this->_pipeline);

        uint32_t idx = 0;
        for (Object *object: this->_engine->scene()->objects()) {
            glm::mat4 model = object->getModelMatrix();
            MeshConstants constants = {
                    .matrix = projection * view * model,
            };
            vkCmdPushConstants(commandBuffer, this->_pipelineLayout, VK_SHADER_STAGE_VERTEX_BIT, 0,
                               sizeof(MeshConstants), &constants);

            VkBuffer vertexBuffer = object->mesh()->vertices->getHandle();
            vkCmdBindVertexBuffers(commandBuffer, 0, 1, &vertexBuffer, &offset);

            if (object->mesh()->indices != nullptr) {
                vkCmdBindIndexBuffer(commandBuffer, object->mesh()->indices->getHandle(), 0, VK_INDEX_TYPE_UINT32);
            }

            if (object->mesh()->indices != nullptr) {
                vkCmdDrawIndexed(commandBuffer, object->mesh()->count, 1, 0, 0, idx++);
            } else {
                vkCmdDraw(commandBuffer, object->mesh()->vertices->getSize(), 1, 0, 0);
            }
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

    this->_pipeline = PipelineBuilder(this->_renderingDevice, this->_renderpass, this->_pipelineLayout)
            .addVertexShader("data/shaders/shadow.vert.spv")
            .addFragmentShader("data/shaders/shadow.frag.spv")
            .addBinding(0, sizeof(Vertex), VK_VERTEX_INPUT_RATE_VERTEX)
            .addAttribute(0, 0, offsetof(Vertex, pos), VK_FORMAT_R32G32B32_SFLOAT)
            .withCullMode(VK_CULL_MODE_NONE)
            .withDepthBias()
            .build();

    for (uint32_t idx = 0; idx < MAX_NUM_LIGHTS; idx++) {
        this->_depthImages[idx] = this->_renderingObjectsFactory->createImageObject(SIZE, SIZE, 1, 0,
                                                                                    this->_renderingDevice->getPhysicalDevice()->getDepthFormat(),
                                                                                    VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT |
                                                                                    VK_IMAGE_USAGE_SAMPLED_BIT,
                                                                                    VK_SAMPLE_COUNT_1_BIT,
                                                                                    VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);
        this->_depthImageViews[idx] = this->_renderingObjectsFactory->createImageViewObject(this->_depthImages[idx],
                                                                                            VK_IMAGE_VIEW_TYPE_2D,
                                                                                            VK_IMAGE_ASPECT_DEPTH_BIT);
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
