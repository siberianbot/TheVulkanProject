#include "SceneRenderpass.hpp"

#include <array>
#include <fstream>

#include "Mesh.hpp"
#include "VulkanCommon.hpp"
#include "VulkanRenderpassBuilder.hpp"

VkShaderModule SceneRenderpass::createShaderModule(const std::string &path) {
    std::ifstream file(path, std::ios::ate | std::ios::binary);
    if (!file.is_open()) {
        throw std::runtime_error("shader file read failure");
    }

    size_t size = file.tellg();
    std::vector<char> buffer(size);

    file.seekg(0);
    file.read(buffer.data(), size);
    file.close();

    VkShaderModuleCreateInfo shaderModuleCreateInfo = {
            .sType = VkStructureType::VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO,
            .pNext = nullptr,
            .flags = 0,
            .codeSize = buffer.size(),
            .pCode = reinterpret_cast<const uint32_t *>(buffer.data())
    };

    VkShaderModule shaderModule;
    vkEnsure(vkCreateShaderModule(this->_renderingDevice.device, &shaderModuleCreateInfo, nullptr, &shaderModule));

    return shaderModule;
}

void SceneRenderpass::createPipeline() {
    VkShaderModule vertexShaderModule = createShaderModule(VK_DEFAULT_VERTEX_SHADER);
    VkShaderModule fragmentShaderModule = createShaderModule(VK_DEFAULT_FRAGMENT_SHADER);

    VkPipelineShaderStageCreateInfo vertexShaderStage = {
            .sType = VkStructureType::VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO,
            .pNext = nullptr,
            .flags = 0,
            .stage = VkShaderStageFlagBits::VK_SHADER_STAGE_VERTEX_BIT,
            .module = vertexShaderModule,
            .pName = "main",
            .pSpecializationInfo = nullptr
    };

    VkPipelineShaderStageCreateInfo fragmentShaderStage = {
            .sType = VkStructureType::VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO,
            .pNext = nullptr,
            .flags = 0,
            .stage = VkShaderStageFlagBits::VK_SHADER_STAGE_FRAGMENT_BIT,
            .module = fragmentShaderModule,
            .pName = "main",
            .pSpecializationInfo = nullptr
    };

    std::array<VkPipelineShaderStageCreateInfo, 2> shaderStages = {
            vertexShaderStage,
            fragmentShaderStage
    };

    VkVertexInputBindingDescription bindingDescription = {
            .binding = 0,
            .stride = sizeof(Vertex),
            .inputRate = VkVertexInputRate::VK_VERTEX_INPUT_RATE_VERTEX
    };

    std::array<VkVertexInputAttributeDescription, 3> attributeDescriptions = {
            VkVertexInputAttributeDescription{
                    .location = 0,
                    .binding = 0,
                    .format = VK_FORMAT_R32G32B32_SFLOAT,
                    .offset = offsetof(Vertex, pos)
            },
            VkVertexInputAttributeDescription{
                    .location = 1,
                    .binding = 0,
                    .format = VK_FORMAT_R32G32B32_SFLOAT,
                    .offset = offsetof(Vertex, color)
            },
            VkVertexInputAttributeDescription{
                    .location = 2,
                    .binding = 0,
                    .format = VK_FORMAT_R32G32_SFLOAT,
                    .offset = offsetof(Vertex, texCoord)
            }
    };

    VkPipelineVertexInputStateCreateInfo vertexInputStateCreateInfo = {
            .sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO,
            .pNext = nullptr,
            .flags = 0,
            .vertexBindingDescriptionCount = 1,
            .pVertexBindingDescriptions = &bindingDescription,
            .vertexAttributeDescriptionCount = static_cast<uint32_t>(attributeDescriptions.size()),
            .pVertexAttributeDescriptions = attributeDescriptions.data()
    };

    VkPipelineInputAssemblyStateCreateInfo inputAssemblyStateCreateInfo = {
            .sType = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO,
            .pNext = nullptr,
            .flags = 0,
            .topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST,
            .primitiveRestartEnable = VK_FALSE
    };

    VkPipelineViewportStateCreateInfo viewportStateCreateInfo = {
            .sType = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO,
            .pNext = nullptr,
            .flags = 0,
            .viewportCount = 1,
            .pViewports = nullptr,
            .scissorCount = 1,
            .pScissors = nullptr
    };

    VkPipelineRasterizationStateCreateInfo rasterizer = {
            .sType = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO,
            .pNext = nullptr,
            .flags = 0,
            .depthClampEnable = VK_FALSE,
            .rasterizerDiscardEnable = VK_FALSE,
            .polygonMode = VK_POLYGON_MODE_FILL,
            .cullMode = VK_CULL_MODE_BACK_BIT,
            .frontFace = VK_FRONT_FACE_COUNTER_CLOCKWISE,
            .depthBiasEnable = VK_FALSE,
            .depthBiasConstantFactor = 0,
            .depthBiasClamp = 0,
            .depthBiasSlopeFactor = 0,
            .lineWidth = 1.0f,
    };

    VkPipelineMultisampleStateCreateInfo multisampling = {
            .sType = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO,
            .pNext = nullptr,
            .flags = 0,
            .rasterizationSamples = this->_renderingDevice.samples,
            .sampleShadingEnable = VK_FALSE,
            .minSampleShading = 0,
            .pSampleMask = nullptr,
            .alphaToCoverageEnable = VK_FALSE,
            .alphaToOneEnable = VK_FALSE
    };

    VkPipelineDepthStencilStateCreateInfo depthStencil = {
            .sType = VK_STRUCTURE_TYPE_PIPELINE_DEPTH_STENCIL_STATE_CREATE_INFO,
            .pNext = nullptr,
            .flags = 0,
            .depthTestEnable = VK_TRUE,
            .depthWriteEnable = VK_TRUE,
            .depthCompareOp = VK_COMPARE_OP_LESS,
            .depthBoundsTestEnable = VK_FALSE,
            .stencilTestEnable = VK_FALSE,
            .front = {},
            .back = {},
            .minDepthBounds = 0,
            .maxDepthBounds = 0
    };

    VkPipelineColorBlendAttachmentState colorBlendAttachment = {
            .blendEnable = VK_FALSE,
            .srcColorBlendFactor = {},
            .dstColorBlendFactor = {},
            .colorBlendOp = VkBlendOp::VK_BLEND_OP_ADD,
            .srcAlphaBlendFactor = {},
            .dstAlphaBlendFactor = {},
            .alphaBlendOp = VkBlendOp::VK_BLEND_OP_ADD,
            .colorWriteMask = VK_COLOR_COMPONENT_R_BIT |
                              VK_COLOR_COMPONENT_G_BIT |
                              VK_COLOR_COMPONENT_B_BIT |
                              VK_COLOR_COMPONENT_A_BIT
    };

    VkPipelineColorBlendStateCreateInfo colorBlending = {
            .sType = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO,
            .pNext = nullptr,
            .flags = 0,
            .logicOpEnable = VK_FALSE,
            .logicOp = VK_LOGIC_OP_COPY,
            .attachmentCount = 1,
            .pAttachments = &colorBlendAttachment,
            .blendConstants = {0, 0, 0, 0}
    };

    std::array<VkDynamicState, 2> dynamicStates = {
            VK_DYNAMIC_STATE_VIEWPORT,
            VK_DYNAMIC_STATE_SCISSOR
    };

    VkPipelineDynamicStateCreateInfo dynamicState = {
            .sType = VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO,
            .pNext = nullptr,
            .flags = 0,
            .dynamicStateCount = static_cast<uint32_t>(dynamicStates.size()),
            .pDynamicStates = dynamicStates.data()
    };

    VkGraphicsPipelineCreateInfo pipelineInfo = {
            .sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO,
            .pNext = nullptr,
            .flags = 0,
            .stageCount = static_cast<uint32_t>(shaderStages.size()),
            .pStages = shaderStages.data(),
            .pVertexInputState = &vertexInputStateCreateInfo,
            .pInputAssemblyState = &inputAssemblyStateCreateInfo,
            .pTessellationState = nullptr,
            .pViewportState = &viewportStateCreateInfo,
            .pRasterizationState = &rasterizer,
            .pMultisampleState = &multisampling,
            .pDepthStencilState = &depthStencil,
            .pColorBlendState = &colorBlending,
            .pDynamicState = &dynamicState,
            .layout = this->_pipelineLayout,
            .renderPass = this->_renderpass,
            .subpass = 0,
            .basePipelineHandle = VK_NULL_HANDLE,
            .basePipelineIndex = 0
    };

    vkEnsure(vkCreateGraphicsPipelines(this->_renderingDevice.device, VK_NULL_HANDLE, 1, &pipelineInfo, nullptr,
                                       &this->_pipeline));

    vkDestroyShaderModule(this->_renderingDevice.device, vertexShaderModule, nullptr);
    vkDestroyShaderModule(this->_renderingDevice.device, fragmentShaderModule, nullptr);
}

SceneRenderpass::SceneRenderpass(const RenderingDevice &renderingDevice, VkPipelineLayout pipelineLayout)
        : RenderpassBase(renderingDevice),
          _pipelineLayout(pipelineLayout) {
    //
}

SceneRenderpass::~SceneRenderpass() {
    if (_pipeline != VK_NULL_HANDLE) {
        vkDestroyPipeline(this->_renderingDevice.device, this->_pipeline, nullptr);
    }
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

        vkCmdBindVertexBuffers(commandBuffer, 0, 1, &mesh->vertexBuffer.buffer, &offset);
        vkCmdBindIndexBuffer(commandBuffer, mesh->indexBuffer.buffer, 0, VK_INDEX_TYPE_UINT32);

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

    createPipeline();
}

void SceneRenderpass::destroyRenderpass() {
    vkDestroyPipeline(this->_renderingDevice.device, this->_pipeline, nullptr);

    RenderpassBase::destroyRenderpass();
}

void SceneRenderpass::addMesh(BoundMeshInfo *mesh) {
    this->_meshes.push_back(mesh);
}

void SceneRenderpass::removeMesh(BoundMeshInfo *mesh) {
    this->_meshes.erase(std::remove(this->_meshes.begin(), this->_meshes.end(), mesh));
}
