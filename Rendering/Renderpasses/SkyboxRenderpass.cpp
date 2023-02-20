#include "SkyboxRenderpass.hpp"

#include "../../Mesh.hpp"

#include "../Common.hpp"

static constexpr const char SKYBOX_TEXTURE_PATH[] = "textures/skybox.png";
static constexpr std::array<Vertex, 36> SKYBOX_TRIANGLES = {
        // up
        Vertex{.pos = {-1, 1, -1}, .color = {1, 1, 1}, .texCoord = {0.25, 0}},
        Vertex{.pos = {1, 1, -1}, .color = {1, 1, 1}, .texCoord = {0.5, 0}},
        Vertex{.pos = {1, 1, 1}, .color = {1, 1, 1}, .texCoord = {0.5, 0.33}},
        Vertex{.pos = {1, 1, 1}, .color = {1, 1, 1}, .texCoord = {0.5, 0.33}},
        Vertex{.pos = {-1, 1, 1}, .color = {1, 1, 1}, .texCoord = {0.25, 0.33}},
        Vertex{.pos = {-1, 1, -1}, .color = {1, 1, 1}, .texCoord = {0.25, 0}},

        // front
        Vertex{.pos = {-1, 1, 1}, .color = {1, 1, 1}, .texCoord = {0.25, 0.33}},
        Vertex{.pos = {1, 1, 1}, .color = {1, 1, 1}, .texCoord = {0.5, 0.33}},
        Vertex{.pos = {1, -1, 1}, .color = {1, 1, 1}, .texCoord = {0.5, 0.66}},
        Vertex{.pos = {1, -1, 1}, .color = {1, 1, 1}, .texCoord = {0.5, 0.66}},
        Vertex{.pos = {-1, -1, 1}, .color = {1, 1, 1}, .texCoord = {0.25, 0.66}},
        Vertex{.pos = {-1, 1, 1}, .color = {1, 1, 1}, .texCoord = {0.25, 0.33}},

        // left
        Vertex{.pos = {-1, 1, -1}, .color = {1, 1, 1}, .texCoord = {0, 0.33}},
        Vertex{.pos = {-1, 1, 1}, .color = {1, 1, 1}, .texCoord = {0.25, 0.33}},
        Vertex{.pos = {-1, -1, 1}, .color = {1, 1, 1}, .texCoord = {0.25, 0.66}},
        Vertex{.pos = {-1, -1, 1}, .color = {1, 1, 1}, .texCoord = {0.25, 0.66}},
        Vertex{.pos = {-1, -1, -1}, .color = {1, 1, 1}, .texCoord = {0, 0.66}},
        Vertex{.pos = {-1, 1, -1}, .color = {1, 1, 1}, .texCoord = {0, 0.33}},

        // right
        Vertex{.pos = {1, 1, 1}, .color = {1, 1, 1}, .texCoord = {0.5, 0.33}},
        Vertex{.pos = {1, 1, -1}, .color = {1, 1, 1}, .texCoord = {0.75, 0.33}},
        Vertex{.pos = {1, -1, -1}, .color = {1, 1, 1}, .texCoord = {0.75, 0.66}},
        Vertex{.pos = {1, -1, -1}, .color = {1, 1, 1}, .texCoord = {0.75, 0.66}},
        Vertex{.pos = {1, -1, 1}, .color = {1, 1, 1}, .texCoord = {0.5, 0.66}},
        Vertex{.pos = {1, 1, 1}, .color = {1, 1, 1}, .texCoord = {0.5, 0.33}},

        // back
        Vertex{.pos = {1, 1, -1}, .color = {1, 1, 1}, .texCoord = {0.75, 0.33}},
        Vertex{.pos = {-1, 1, -1}, .color = {1, 1, 1}, .texCoord = {1, 0.33}},
        Vertex{.pos = {-1, -1, -1}, .color = {1, 1, 1}, .texCoord = {1, 0.66}},
        Vertex{.pos = {-1, -1, -1}, .color = {1, 1, 1}, .texCoord = {1, 0.66}},
        Vertex{.pos = {1, -1, -1}, .color = {1, 1, 1}, .texCoord = {0.75, 0.66}},
        Vertex{.pos = {1, 1, -1}, .color = {1, 1, 1}, .texCoord = {0.75, 0.33}},

        // down
        Vertex{.pos = {-1, -1, 1}, .color = {1, 1, 1}, .texCoord = {0.25, 0.66}},
        Vertex{.pos = {1, -1, 1}, .color = {1, 1, 1}, .texCoord = {0.5, 0.66}},
        Vertex{.pos = {1, -1, -1}, .color = {1, 1, 1}, .texCoord = {0.5, 1}},
        Vertex{.pos = {1, -1, -1}, .color = {1, 1, 1}, .texCoord = {0.5, 1}},
        Vertex{.pos = {-1, -1, -1}, .color = {1, 1, 1}, .texCoord = {0.25, 1}},
        Vertex{.pos = {-1, -1, 1}, .color = {1, 1, 1}, .texCoord = {0.25, 0.66}},
};

SkyboxRenderpass::SkyboxRenderpass(const DeviceData &deviceData)
        : RenderpassBase(0, deviceData) {
    //
}

void SkyboxRenderpass::recordCommands(VkCommandBuffer commandBuffer, VkRect2D renderArea,
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

    VkDeviceSize offset = 0;
//    VkDescriptorSet descriptorSet = this->_descriptorSets[frameIdx];
//
//    Constants constants = {
//            .model = glm::mat4(1)
//    };
//    vkCmdPushConstants(commandBuffer, this->_pipelineLayout,
//                       VK_SHADER_STAGE_VERTEX_BIT, 0, sizeof(Constants), &constants);

    vkCmdBindVertexBuffers(commandBuffer, 0, 1, &this->_vertexBuffer.buffer, &offset);
    vkCmdDraw(commandBuffer, static_cast<uint32_t>(SKYBOX_TRIANGLES.size()), 0, 0, 0);

    vkCmdEndRenderPass(commandBuffer);
}

void SkyboxRenderpass::initRenderpass() {
    RenderpassBase::initRenderpass();

    VkDeviceSize size = sizeof(SKYBOX_TRIANGLES[0]) * SKYBOX_TRIANGLES.size();

    this->_vertexBuffer.buffer = createBuffer(this->_deviceData.device, size, VK_BUFFER_USAGE_TRANSFER_SRC_BIT |
                                                                              VK_BUFFER_USAGE_VERTEX_BUFFER_BIT);
    this->_vertexBuffer.memory = allocateMemoryForBuffer(this->_deviceData.device, this->_deviceData.physicalDevice,
                                                         this->_vertexBuffer.buffer,
                                                         VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT |
                                                         VK_MEMORY_PROPERTY_HOST_COHERENT_BIT |
                                                         VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);

    void *data;
    vkMapMemory(this->_deviceData.device, this->_vertexBuffer.memory, 0, size, 0, &data);
    memcpy(data, SKYBOX_TRIANGLES.data(), size);
    vkUnmapMemory(this->_deviceData.device, this->_vertexBuffer.memory);
}
