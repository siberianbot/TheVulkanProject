#ifndef RENDERING_STAGES_RENDERSTAGE_HPP
#define RENDERING_STAGES_RENDERSTAGE_HPP

#include <memory>

#include <vulkan/vulkan.hpp>

class Swapchain;

struct RenderStageInitContext {
    std::shared_ptr<Swapchain> swapchain;
};

class RenderStage {
protected:
    bool _initialized;

public:
    virtual ~RenderStage() = default;

    virtual void init(const RenderStageInitContext &context) = 0;
    virtual void destroy() = 0;

    virtual void draw(uint32_t imageIdx, const vk::CommandBuffer &commandBuffer) = 0;

    [[nodiscard]] const bool &isInitialized() const { return this->_initialized; }
};

#endif // RENDERING_STAGES_RENDERSTAGE_HPP
