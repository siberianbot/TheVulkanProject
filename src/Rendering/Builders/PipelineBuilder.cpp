#include "PipelineBuilder.hpp"

#include <fstream>

#include "src/Rendering/Common.hpp"
#include "src/Rendering/RenderingDevice.hpp"
#include "src/Rendering/Builders/SpecializationInfoBuilder.hpp"

VkShaderModule PipelineBuilder::createShaderModule(const std::string &path) {
    std::ifstream file(path, std::ios::ate | std::ios::binary);
    if (!file.is_open()) {
        throw std::runtime_error("shader file read failure");
    }

    size_t size = file.tellg();
    std::vector<char> buffer(size);

    file.seekg(0);
    file.read(buffer.data(), size);
    file.close();

    return this->_renderingDevice->createShaderModule(buffer);
}

PipelineBuilder::PipelineBuilder(RenderingDevice *renderingDevice,
                                 VkRenderPass renderpass, VkPipelineLayout pipelineLayout)
        : _renderingDevice(renderingDevice),
          _renderpass(renderpass),
          _pipelineLayout(pipelineLayout) {
    //
}

PipelineBuilder::~PipelineBuilder() {
    if (this->_vertexShader != VK_NULL_HANDLE) {
        this->_renderingDevice->destroyShaderModule(this->_vertexShader);
    }

    if (this->_fragmentShader != VK_NULL_HANDLE) {
        this->_renderingDevice->destroyShaderModule(this->_fragmentShader);
    }
}

PipelineBuilder &PipelineBuilder::addVertexShader(const std::string &path) {
    this->_vertexShader = createShaderModule(path);

    return *this;
}

PipelineBuilder &PipelineBuilder::addFragmentShader(const std::string &path) {
    this->_fragmentShader = createShaderModule(path);

    return *this;
}

PipelineBuilder &PipelineBuilder::addBinding(uint32_t bindingIdx, uint32_t stride,
                                             VkVertexInputRate inputRate) {
    this->_bindings.push_back(VkVertexInputBindingDescription{
            .binding = bindingIdx,
            .stride = stride,
            .inputRate = inputRate
    });

    return *this;
}

PipelineBuilder &PipelineBuilder::addAttribute(uint32_t bindingIdx, uint32_t locationIdx,
                                               uint32_t offset, VkFormat format) {
    this->_attributes.push_back(VkVertexInputAttributeDescription{
            .location = locationIdx,
            .binding = bindingIdx,
            .format = format,
            .offset = offset
    });

    return *this;
}

PipelineBuilder &PipelineBuilder::withVertexShaderSpecialization(ShaderSpecializationFunc func) {
    SpecializationInfoBuilder builder;

    func(builder);

    this->_vertexShaderSpecialization = builder.build();

    return *this;
}

PipelineBuilder &PipelineBuilder::withFragmentShaderSpecialization(ShaderSpecializationFunc func) {
    SpecializationInfoBuilder builder;

    func(builder);

    this->_fragmentShaderSpecialization = builder.build();

    return *this;
}

PipelineBuilder &PipelineBuilder::withDepthBias() {
    this->_depthBiasEnabled = true;

    return *this;
}

PipelineBuilder &PipelineBuilder::withCullMode(VkCullModeFlags cullMode) {
    this->_cullMode = cullMode;

    return *this;
}

PipelineBuilder &PipelineBuilder::withRasterizationSamples(VkSampleCountFlagBits samples) {
    this->_rasterizationSamples = samples;

    return *this;
}

PipelineBuilder &PipelineBuilder::withColorBlendAttachmentCount(uint32_t count) {
    this->_colorBlendAttachmentCount = count;

    return *this;
}

PipelineBuilder &PipelineBuilder::forSubpass(uint32_t subpass) {
    this->_subpassIdx = subpass;

    return *this;
}

VkPipeline PipelineBuilder::build() {
    if (this->_vertexShader == VK_NULL_HANDLE || this->_fragmentShader == VK_NULL_HANDLE) {
        throw std::runtime_error("Vertex and fragment shaders are required");
    }

    const std::array<VkPipelineShaderStageCreateInfo, 2> shaderStages = {
            VkPipelineShaderStageCreateInfo{
                    .sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO,
                    .pNext = nullptr,
                    .flags = 0,
                    .stage = VK_SHADER_STAGE_VERTEX_BIT,
                    .module = this->_vertexShader,
                    .pName = "main",
                    .pSpecializationInfo = this->_vertexShaderSpecialization.has_value()
                    ? &this->_vertexShaderSpecialization.value()
                    : nullptr
            },
            VkPipelineShaderStageCreateInfo{
                    .sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO,
                    .pNext = nullptr,
                    .flags = 0,
                    .stage = VK_SHADER_STAGE_FRAGMENT_BIT,
                    .module = this->_fragmentShader,
                    .pName = "main",
                    .pSpecializationInfo = this->_fragmentShaderSpecialization.has_value()
                    ? &this->_fragmentShaderSpecialization.value()
                    : nullptr
            }
    };

    const VkPipelineVertexInputStateCreateInfo vertexInputStateCreateInfo = {
            .sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO,
            .pNext = nullptr,
            .flags = 0,
            .vertexBindingDescriptionCount = static_cast<uint32_t>(this->_bindings.size()),
            .pVertexBindingDescriptions = this->_bindings.data(),
            .vertexAttributeDescriptionCount = static_cast<uint32_t>(this->_attributes.size()),
            .pVertexAttributeDescriptions = this->_attributes.data()
    };

    const VkPipelineInputAssemblyStateCreateInfo inputAssemblyStateCreateInfo = {
            .sType = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO,
            .pNext = nullptr,
            .flags = 0,
            .topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST,
            .primitiveRestartEnable = VK_FALSE
    };

    const VkPipelineViewportStateCreateInfo viewportStateCreateInfo = {
            .sType = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO,
            .pNext = nullptr,
            .flags = 0,
            .viewportCount = 1,
            .pViewports = nullptr,
            .scissorCount = 1,
            .pScissors = nullptr
    };

    const VkPipelineRasterizationStateCreateInfo rasterizer = {
            .sType = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO,
            .pNext = nullptr,
            .flags = 0,
            .depthClampEnable = VK_FALSE,
            .rasterizerDiscardEnable = VK_FALSE,
            .polygonMode = VK_POLYGON_MODE_FILL,
            .cullMode = this->_cullMode.value_or(VK_CULL_MODE_BACK_BIT),
            .frontFace = VK_FRONT_FACE_COUNTER_CLOCKWISE,
            .depthBiasEnable = this->_depthBiasEnabled ? VK_TRUE : VK_FALSE,
            .depthBiasConstantFactor = 0,
            .depthBiasClamp = 0,
            .depthBiasSlopeFactor = 0,
            .lineWidth = 1.0f,
    };

    const VkPipelineMultisampleStateCreateInfo multisampling = {
            .sType = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO,
            .pNext = nullptr,
            .flags = 0,
            .rasterizationSamples = this->_rasterizationSamples.value_or(VK_SAMPLE_COUNT_1_BIT),
            .sampleShadingEnable = VK_FALSE,
            .minSampleShading = 0,
            .pSampleMask = nullptr,
            .alphaToCoverageEnable = VK_FALSE,
            .alphaToOneEnable = VK_FALSE
    };

    const VkPipelineDepthStencilStateCreateInfo depthStencil = {
            .sType = VK_STRUCTURE_TYPE_PIPELINE_DEPTH_STENCIL_STATE_CREATE_INFO,
            .pNext = nullptr,
            .flags = 0,
            .depthTestEnable = VK_TRUE,
            .depthWriteEnable = VK_TRUE,
            .depthCompareOp = VK_COMPARE_OP_LESS_OR_EQUAL,
            .depthBoundsTestEnable = VK_FALSE,
            .stencilTestEnable = VK_FALSE,
            .front = {},
            .back = {},
            .minDepthBounds = 0,
            .maxDepthBounds = 0
    };

    const VkPipelineColorBlendAttachmentState colorBlendAttachment = {
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

    std::vector<VkPipelineColorBlendAttachmentState> colorBlendAttachments(
            this->_colorBlendAttachmentCount.value_or(1),
            colorBlendAttachment);

    const VkPipelineColorBlendStateCreateInfo colorBlending = {
            .sType = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO,
            .pNext = nullptr,
            .flags = 0,
            .logicOpEnable = VK_FALSE,
            .logicOp = VK_LOGIC_OP_COPY,
            .attachmentCount = static_cast<uint32_t>(colorBlendAttachments.size()),
            .pAttachments = colorBlendAttachments.data(),
            .blendConstants = {0, 0, 0, 0}
    };

    std::vector<VkDynamicState> dynamicStates = {
            VK_DYNAMIC_STATE_VIEWPORT,
            VK_DYNAMIC_STATE_SCISSOR
    };

    if (this->_depthBiasEnabled) {
        dynamicStates.push_back(VK_DYNAMIC_STATE_DEPTH_BIAS);
    }

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
            .subpass = this->_subpassIdx,
            .basePipelineHandle = VK_NULL_HANDLE,
            .basePipelineIndex = 0
    };

    return this->_renderingDevice->createPipeline(&pipelineInfo);
}
