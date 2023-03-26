#include "RenderingLayoutsManager.hpp"

#include "src/Engine/EngineVars.hpp"
#include "src/Rendering/VulkanObjectsAllocator.hpp"
#include "src/Rendering/Builders/DescriptorPoolBuilder.hpp"
#include "src/Rendering/Builders/DescriptorSetLayoutBuilder.hpp"
#include "src/Rendering/Builders/PipelineLayoutBuilder.hpp"
#include "src/Rendering/Types/MeshConstants.hpp"

RenderingLayoutsManager::RenderingLayoutsManager(
        const std::shared_ptr<EngineVars> &engineVars,
        const std::shared_ptr<RenderingDevice> &renderingDevice,
        const std::shared_ptr<VulkanObjectsAllocator> &vulkanObjectsAllocator)
        : _engineVars(engineVars),
          _renderingDevice(renderingDevice),
          _vulkanObjectsAllocator(vulkanObjectsAllocator) {
    //
}

void RenderingLayoutsManager::init() {
    uint32_t shadowMapCount = this->_engineVars->getOrDefault(RENDERING_SCENE_STAGE_SHADOW_MAP_COUNT, 32)->intValue;
    uint32_t lightCount = this->_engineVars->getOrDefault(RENDERING_SCENE_STAGE_LIGHT_COUNT, 128)->intValue;

    this->_descriptorPool = DescriptorPoolBuilder(this->_vulkanObjectsAllocator)
            .forType(VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER)
            .forType(VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER)
            .build();

    // 0 - albedo texture
    // 1 - specular texture
    this->_modelDescriptorSetLayout = DescriptorSetLayoutBuilder(this->_vulkanObjectsAllocator)
            .withBinding(0, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, VK_SHADER_STAGE_FRAGMENT_BIT)
            .withBinding(1, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, VK_SHADER_STAGE_FRAGMENT_BIT)
            .build();

    // 0 - albedo
    // 1 - position
    // 2 - normal
    // 3 - specular
    // 4 - shadow map images
    // 5 - shadow map data
    // 6 - light data
    // 7 - camera data
    // 8 - scene data
    this->_compositionDescriptorSetLayout = DescriptorSetLayoutBuilder(this->_vulkanObjectsAllocator)
            .withBinding(0, VK_DESCRIPTOR_TYPE_INPUT_ATTACHMENT, VK_SHADER_STAGE_FRAGMENT_BIT)
            .withBinding(1, VK_DESCRIPTOR_TYPE_INPUT_ATTACHMENT, VK_SHADER_STAGE_FRAGMENT_BIT)
            .withBinding(2, VK_DESCRIPTOR_TYPE_INPUT_ATTACHMENT, VK_SHADER_STAGE_FRAGMENT_BIT)
            .withBinding(3, VK_DESCRIPTOR_TYPE_INPUT_ATTACHMENT, VK_SHADER_STAGE_FRAGMENT_BIT)
            .withBinding(4, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, VK_SHADER_STAGE_FRAGMENT_BIT, shadowMapCount)
            .withBinding(5, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, VK_SHADER_STAGE_FRAGMENT_BIT, shadowMapCount)
            .withBinding(6, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, VK_SHADER_STAGE_FRAGMENT_BIT, lightCount)
            .withBinding(7, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, VK_SHADER_STAGE_FRAGMENT_BIT)
            .withBinding(8, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, VK_SHADER_STAGE_FRAGMENT_BIT)
            .build();

    this->_shadowPipelineLayout = PipelineLayoutBuilder(this->_vulkanObjectsAllocator)
            .withPushConstant(VK_SHADER_STAGE_VERTEX_BIT, 0, sizeof(MeshConstants))
            .build();

    this->_modelPipelineLayout = PipelineLayoutBuilder(this->_vulkanObjectsAllocator)
            .withPushConstant(VK_SHADER_STAGE_VERTEX_BIT, 0, sizeof(MeshConstants))
            .withDescriptorSetLayout(this->_modelDescriptorSetLayout)
            .build();

    this->_compositionPipelineLayout = PipelineLayoutBuilder(this->_vulkanObjectsAllocator)
            .withDescriptorSetLayout(this->_compositionDescriptorSetLayout)
            .build();
}

void RenderingLayoutsManager::destroy() {
    this->_vulkanObjectsAllocator->destroyPipelineLayout(this->_compositionPipelineLayout);
    this->_vulkanObjectsAllocator->destroyPipelineLayout(this->_modelPipelineLayout);
    this->_vulkanObjectsAllocator->destroyPipelineLayout(this->_shadowPipelineLayout);
    this->_vulkanObjectsAllocator->destroyDescriptorSetLayout(this->_compositionDescriptorSetLayout);
    this->_vulkanObjectsAllocator->destroyDescriptorSetLayout(this->_modelDescriptorSetLayout);
    this->_vulkanObjectsAllocator->destroyDescriptorPool(this->_descriptorPool);
}
