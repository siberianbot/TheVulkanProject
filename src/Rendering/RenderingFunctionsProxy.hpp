#ifndef RENDERING_FUNCTIONS_HPP
#define RENDERING_FUNCTIONS_HPP

#include <memory>

#include <vulkan/vulkan.hpp>

class PhysicalDevice;
class RenderingDevice;

class RenderingFunctionsProxy {
private:
    std::shared_ptr<PhysicalDevice> _physicalDevice;
    std::shared_ptr<RenderingDevice> _renderingDevice;

public:
    RenderingFunctionsProxy(const std::shared_ptr<PhysicalDevice> &physicalDevice,
                            const std::shared_ptr<RenderingDevice> &renderingDevice);

    static std::shared_ptr<RenderingFunctionsProxy> create(const std::shared_ptr<PhysicalDevice> &physicalDevice,
                                                           const std::shared_ptr<RenderingDevice> &renderingDevice);

    VkCommandPool createCommandPool();
    void destroyCommandPool(VkCommandPool commandPool);
};

#endif // RENDERING_FUNCTIONS_HPP
