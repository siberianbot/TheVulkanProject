#ifndef RENDERING_OBJECTS_DESCRIPTORSETOBJECT_HPP
#define RENDERING_OBJECTS_DESCRIPTORSETOBJECT_HPP

#include <map>
#include <memory>

#include <vulkan/vulkan.hpp>

class RenderingDevice;
class BufferObject;
class ImageViewObject;

struct DescriptorSetBinding {
    std::shared_ptr<BufferObject> buffer;
    std::shared_ptr<ImageViewObject> imageView;
};

class DescriptorSetObject {
private:
    std::shared_ptr<RenderingDevice> _renderingDevice;

    VkDescriptorPool _descriptorPool;
    VkDescriptorSet _descriptorSet;

    std::map<uint32_t, DescriptorSetBinding> _bindings;

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

    [[nodiscard]] const std::map<uint32_t, DescriptorSetBinding> &bindings() const { return this->_bindings; }

    static std::shared_ptr<DescriptorSetObject> create(const std::shared_ptr<RenderingDevice> &renderingDevice,
                                                       VkDescriptorPool descriptorPool, VkDescriptorSetLayout layout);
};


#endif // RENDERING_OBJECTS_DESCRIPTORSETOBJECT_HPP
