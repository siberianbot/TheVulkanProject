#ifndef RENDERING_OBJECTS_DESCRIPTORSETOBJECT_HPP
#define RENDERING_OBJECTS_DESCRIPTORSETOBJECT_HPP

#include <memory>

#include <vulkan/vulkan.hpp>

class RenderingDevice;
class BufferObject;
class ImageViewObject;

class DescriptorSetObject {
private:
    std::shared_ptr<RenderingDevice> _renderingDevice;

    VkDescriptorPool _descriptorPool;
    VkDescriptorSet _descriptorSet;

public:
    DescriptorSetObject(const std::shared_ptr<RenderingDevice> &renderingDevice,
                        VkDescriptorPool descriptorPool, VkDescriptorSet descriptorSet);

    [[nodiscard]] VkDescriptorSet getHandle() const { return this->_descriptorSet; }

    void updateWithBuffer(uint32_t bindingIdx, VkDescriptorType descriptorType,
                          const std::shared_ptr<BufferObject> buffer, uint32_t offset, uint32_t range);
    void updateWithImageView(uint32_t bindingIdx, VkDescriptorType descriptorType,
                             const std::shared_ptr<ImageViewObject> imageView,
                             VkImageLayout imageLayout, VkSampler sampler = VK_NULL_HANDLE);

    void destroy();

    static std::shared_ptr<DescriptorSetObject> create(const std::shared_ptr<RenderingDevice> &renderingDevice,
                                                       VkDescriptorPool descriptorPool, VkDescriptorSetLayout layout);
};


#endif // RENDERING_OBJECTS_DESCRIPTORSETOBJECT_HPP
