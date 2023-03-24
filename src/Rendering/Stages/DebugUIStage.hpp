#ifndef RENDERING_STAGES_DEBUGUISTAGE_HPP
#define RENDERING_STAGES_DEBUGUISTAGE_HPP

#include <memory>

#include "src/Rendering/Stages/StageBase.hpp"

class EventQueue;
class RenderingManager;
class DebugUIRenderpass;

class DebugUIStage : public StageBase {
private:
    std::shared_ptr<EventQueue> _eventQueue;
    std::shared_ptr<RenderingManager> _renderingManager;

    VkDescriptorPool _descriptorPool;
    std::unique_ptr<DebugUIRenderpass> _debugUIRenderpass;

public:
    DebugUIStage(const std::shared_ptr<EventQueue> &eventQueue,
                 const std::shared_ptr<RenderingManager> &renderingManager);
    ~DebugUIStage() override = default;

    void init() override;
    void destroy() override;

    void record(VkCommandBuffer commandBuffer, uint32_t frameIdx, uint32_t imageIdx) override;
};

#endif // RENDERING_STAGES_DEBUGUISTAGE_HPP
