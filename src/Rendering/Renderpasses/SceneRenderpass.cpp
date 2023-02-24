#include "SceneRenderpass.hpp"

#include "src/Engine.hpp"
#include "src/Scene/Object.hpp"
#include "src/Scene/Scene.hpp"
#include "src/Mesh.hpp"
#include "src/Rendering/RenderingDevice.hpp"
#include "src/Rendering/RenderingObjectsFactory.hpp"
#include "src/Rendering/Swapchain.hpp"
#include "src/Rendering/RenderpassBuilder.hpp"
#include "src/Rendering/PipelineBuilder.hpp"
#include "src/Rendering/Objects/BufferObject.hpp"
#include "src/Rendering/Objects/ImageObject.hpp"
#include "src/Rendering/Objects/ImageViewObject.hpp"
#include "src/Rendering/Objects/DescriptorSetObject.hpp"
#include "src/Rendering/Objects/RenderingLayoutObject.hpp"

#define GLM_FORCE_RADIANS
#define GLM_FORCE_DEPTH_ZERO_TO_ONE
#define GLM_ENABLE_EXPERIMENTAL

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

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
    this->_textureSampler = this->_renderingDevice->createSampler();
    this->_renderingLayoutObject = renderingObjectsFactory->createRenderingLayoutObject();
}

SceneRenderpass::~SceneRenderpass() {
    for (auto &it: this->_renderData) {
        delete it.second.descriptorSet;
        delete it.second.textureView;
    }

    delete this->_renderingLayoutObject;
    this->_renderingDevice->destroySampler(this->_textureSampler);
}

void SceneRenderpass::recordCommands(VkCommandBuffer commandBuffer, VkRect2D renderArea,
                                     uint32_t frameIdx, uint32_t imageIdx) {
    VkExtent2D extent = this->_swapchain->getSwapchainExtent();
    VkPipelineLayout pipelineLayout = this->_renderingLayoutObject->getPipelineLayout();

    SceneData *sceneData = this->_renderingLayoutObject->getSceneData(frameIdx);
    sceneData->view = this->_engine->camera().view();
    sceneData->projection = glm::perspective(glm::radians(45.0f), extent.width / (float) extent.height, 0.1f, 100.0f);
    sceneData->projection[1][1] *= -1;

    VkDescriptorSet sceneDataDescriptorSet = this->_renderingLayoutObject->getSceneDataDescriptorSetObject()
            ->getDescriptorSet(frameIdx);

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

    vkCmdBindPipeline(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, this->_pipeline);

    vkCmdBindDescriptorSets(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, pipelineLayout,
                            0, 1, &sceneDataDescriptorSet, 0, nullptr);

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

    uint32_t idx = 0;
    for (Object *object: this->_engine->scene()->objects()) {
        RenderData renderData = getRenderData(object);

        VkDeviceSize offset = 0;
        VkDescriptorSet descriptorSet = renderData.descriptorSet->getDescriptorSet(frameIdx);

        MeshConstants constants = {
                .model = object->getModelMatrix()
        };
        vkCmdPushConstants(commandBuffer, pipelineLayout, VK_SHADER_STAGE_VERTEX_BIT, 0, sizeof(MeshConstants),
                           &constants);

        VkBuffer vertexBuffer = object->mesh()->vertices->getHandle();
        vkCmdBindVertexBuffers(commandBuffer, 0, 1, &vertexBuffer, &offset);
        vkCmdBindIndexBuffer(commandBuffer, object->mesh()->indices->getHandle(), 0, VK_INDEX_TYPE_UINT32);

        vkCmdBindDescriptorSets(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, pipelineLayout,
                                1, 1, &descriptorSet, 0, nullptr);

        vkCmdDrawIndexed(commandBuffer, object->mesh()->indicesCount, 1, 0, 0, idx++);
    }

    vkCmdEndRenderPass(commandBuffer);
}

void SceneRenderpass::initRenderpass() {
    this->_renderpass = RenderpassBuilder(this->_renderingDevice)
            .load()
            .build();

    this->_pipeline = PipelineBuilder(this->_renderingDevice, this->_renderpass,
                                      this->_renderingLayoutObject->getPipelineLayout())
            .addVertexShader(DEFAULT_VERTEX_SHADER)
            .addFragmentShader(DEFAULT_FRAGMENT_SHADER)
            .addBinding(0, sizeof(Vertex), VK_VERTEX_INPUT_RATE_VERTEX)
            .addAttribute(0, 0, offsetof(Vertex, pos), VK_FORMAT_R32G32B32_SFLOAT)
            .addAttribute(0, 1, offsetof(Vertex, color), VK_FORMAT_R32G32B32_SFLOAT)
            .addAttribute(0, 2, offsetof(Vertex, texCoord), VK_FORMAT_R32G32_SFLOAT)
            .build();
}

void SceneRenderpass::destroyRenderpass() {
    vkDestroyPipeline(this->_renderingDevice->getHandle(), this->_pipeline, nullptr);

    RenderpassBase::destroyRenderpass();
}
