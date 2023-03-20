#ifndef RENDERING_OBJECTS_DESCRIPTORSETOBJECT_HPP
#define RENDERING_OBJECTS_DESCRIPTORSETOBJECT_HPP

#include <memory>

#include <vulkan/vulkan.hpp>

class RenderingDevice;

class DescriptorSetObject {
private:
    std::shared_ptr<RenderingDevice> _renderingDevice;

    VkDescriptorPool _descriptorPool;
    VkDescriptorSet _descriptorSet;

public:
    DescriptorSetObject(const std::shared_ptr<RenderingDevice> &renderingDevice,
                        VkDescriptorPool descriptorPool, VkDescriptorSet descriptorSet);

    [[nodiscard]] VkDescriptorSet getHandle() const { return this->_descriptorSet; }

    void destroy();

    static std::shared_ptr<DescriptorSetObject> create(const std::shared_ptr<RenderingDevice> &renderingDevice,
                                                       VkDescriptorPool descriptorPool, VkDescriptorSetLayout layout);
};


#endif // RENDERING_OBJECTS_DESCRIPTORSETOBJECT_HPP
