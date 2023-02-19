#include "SceneRenderpass.hpp"

#include <fstream>

#include "Mesh.hpp"
#include "VulkanCommon.hpp"
#include "Rendering/VulkanRenderpassBuilder.hpp"
#include "Rendering/VulkanPipelineBuilder.hpp"

static constexpr const char *DEFAULT_VERTEX_SHADER = "shaders/default.vert.spv";
static constexpr const char *DEFAULT_FRAGMENT_SHADER = "shaders/default.frag.spv";

SceneRenderpass::SceneRenderpass(RenderingDevice *renderingDevice, Swapchain *swapchain,
                                 VkPipelineLayout pipelineLayout)
        : RenderpassBase(renderingDevice, swapchain),
          _pipelineLayout(pipelineLayout) {
    //
}

void SceneRenderpass::recordCommands(VkCommandBuffer commandBuffer, VkRect2D renderArea,
                                     uint32_t frameIdx, uint32_t imageIdx) {
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

    int idx = 0;
    for (BoundMeshInfo *mesh: this->_meshes) {
        VkDeviceSize offset = 0;
        VkDescriptorSet descriptorSet = mesh->descriptorSets[frameIdx];

        Constants constants = {
                .model = mesh->model
        };
        vkCmdPushConstants(commandBuffer, this->_pipelineLayout,
                           VK_SHADER_STAGE_VERTEX_BIT, 0, sizeof(Constants), &constants);

        VkBuffer vertexBuffer = mesh->vertexBuffer->getHandle();
        vkCmdBindVertexBuffers(commandBuffer, 0, 1, &vertexBuffer, &offset);
        vkCmdBindIndexBuffer(commandBuffer, mesh->indexBuffer->getHandle(), 0, VK_INDEX_TYPE_UINT32);

        vkCmdBindDescriptorSets(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, this->_pipelineLayout,
                                0, 1, &descriptorSet, 0, nullptr);

        vkCmdDrawIndexed(commandBuffer, mesh->indicesCount, 1, 0, 0, idx++);
    }

    vkCmdEndRenderPass(commandBuffer);
}

void SceneRenderpass::initRenderpass() {
    this->_renderpass = VulkanRenderpassBuilder(this->_renderingDevice)
            .load()
            .build();

    this->_pipeline = VulkanPipelineBuilder(this->_renderingDevice, this->_renderpass, this->_pipelineLayout)
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

void SceneRenderpass::addMesh(BoundMeshInfo *mesh) {
    this->_meshes.push_back(mesh);
}

void SceneRenderpass::removeMesh(BoundMeshInfo *mesh) {
    this->_meshes.erase(std::remove(this->_meshes.begin(), this->_meshes.end(), mesh));
}
