#ifndef RENDERING_STAGES_STAGE_HPP
#define RENDERING_STAGES_STAGE_HPP

#include <vulkan/vulkan.hpp>

class StageBase {
public:
    virtual ~StageBase() = default;

    virtual void init() = 0;
    virtual void destroy() = 0;

    virtual void record(VkCommandBuffer commandBuffer, uint32_t frameIdx, uint32_t imageIdx) = 0;
};

#endif // RENDERING_STAGES_STAGE_HPP
