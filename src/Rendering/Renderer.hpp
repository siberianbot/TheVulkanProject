#ifndef RENDERING_RENDERER_HPP
#define RENDERING_RENDERER_HPP

#include <map>
#include <memory>
#include <optional>

#include "src/Rendering/Graph/RenderGraph.hpp"

class Log;
class VarCollection;
class GpuManager;
class RenderThread;
class Swapchain;
class RenderStage;
class Window;

class Renderer {
private:
    std::shared_ptr<Log> _log;
    std::shared_ptr<VarCollection> _varCollection;
    std::shared_ptr<GpuManager> _gpuManager;
    std::shared_ptr<Window> _window;

    std::shared_ptr<Swapchain> _swapchain;
    std::shared_ptr<RenderThread> _renderThread;

    std::optional<RenderGraph> _renderGraph;
    std::map<RenderStageRef, std::shared_ptr<RenderStage>> _renderStages;

public:
    Renderer(const std::shared_ptr<Log> &log,
             const std::shared_ptr<VarCollection> &varCollection,
             const std::shared_ptr<GpuManager> &gpuManager,
             const std::shared_ptr<Window> &window);

    void init();
    void destroy();

    void removeRenderGraph();
    void setRenderGraph(const RenderGraph &graph);

    [[nodiscard]] const std::optional<RenderGraph> &getRenderGraph() const { return this->_renderGraph; }

    void addRenderStage(const RenderStageRef &stageRef, const std::shared_ptr<RenderStage> &stage);
    [[nodiscard]] const std::optional<std::shared_ptr<RenderStage>> tryGetRenderStage(
            const RenderStageRef &stageRef);
};

#endif // RENDERING_RENDERER_HPP
