#ifndef RENDERING_RENDERINGLAYOUTSMANAGER_HPP
#define RENDERING_RENDERINGLAYOUTSMANAGER_HPP

#include <memory>

#include <vulkan/vulkan.hpp>

class VarCollection;
class RenderingDevice;
class VulkanObjectsAllocator;
class DescriptorSetObject;

class RenderingLayoutsManager {
private:
    std::shared_ptr<VarCollection> _vars;
    std::shared_ptr<RenderingDevice> _renderingDevice;
    std::shared_ptr<VulkanObjectsAllocator> _vulkanObjectsAllocator;

    VkDescriptorPool _descriptorPool;
    VkDescriptorSetLayout _modelDescriptorSetLayout;
    VkDescriptorSetLayout _compositionDescriptorSetLayout;
    VkPipelineLayout _shadowPipelineLayout;
    VkPipelineLayout _modelPipelineLayout;
    VkPipelineLayout _compositionPipelineLayout;

public:
    RenderingLayoutsManager(const std::shared_ptr<VarCollection> &vars,
                            const std::shared_ptr<RenderingDevice> &renderingDevice,
                            const std::shared_ptr<VulkanObjectsAllocator> &vulkanObjectsAllocator);

    void init();
    void destroy();

    [[nodiscard]] VkDescriptorPool descriptorPool() const { return this->_descriptorPool; }

    [[nodiscard]] VkDescriptorSetLayout modelDescriptorSetLayout() const { return this->_modelDescriptorSetLayout; }
    [[nodiscard]] VkDescriptorSetLayout compositionDescriptorSetLayout() const {
        return this->_compositionDescriptorSetLayout;
    }

    [[nodiscard]] VkPipelineLayout shadowPipelineLayout() const { return this->_shadowPipelineLayout; }
    [[nodiscard]] VkPipelineLayout modelPipelineLayout() const { return this->_modelPipelineLayout; }
    [[nodiscard]] VkPipelineLayout compositionPipelineLayout() const { return this->_compositionPipelineLayout; }
};

#endif // RENDERING_RENDERINGLAYOUTSMANAGER_HPP
