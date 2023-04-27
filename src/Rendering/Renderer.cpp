#include "Renderer.hpp"

#include "src/Engine/EngineError.hpp"
#include "src/Rendering/GpuManager.hpp"
#include "src/Rendering/RenderThread.hpp"
#include "src/Rendering/Swapchain.hpp"
#include "src/Rendering/SwapchainManager.hpp"
#include "src/Rendering/Graph/RenderStage.hpp"

Renderer::Renderer(const std::shared_ptr<Log> &log,
                   const std::shared_ptr<VarCollection> &varCollection,
                   const std::shared_ptr<GpuManager> &gpuManager,
                   const std::shared_ptr<Window> &window)
        : _log(log),
          _varCollection(varCollection),
          _gpuManager(gpuManager),
          _window(window) {
    //
}

void Renderer::init() {
    if (this->_gpuManager->getLogicalDeviceProxy().expired() ||
        this->_gpuManager->getAllocator().expired() ||
        this->_gpuManager->getCommandManager().expired() ||
        this->_gpuManager->getSwapchainManager().expired()) {
        throw EngineError("GPU manager is not initialized");
    }

    auto swapchainManager = this->_gpuManager->getSwapchainManager().lock();
    this->_swapchain = swapchainManager->getSwapchainFor(this->_window);

    for (const auto &[stageRef, stage]: this->_renderStages) {
        stage->init();
    }

    this->_renderThread = std::make_shared<RenderThread>(this,
                                                         this->_log,
                                                         this->_varCollection,
                                                         this->_gpuManager->getCommandManager().lock(),
                                                         this->_gpuManager->getAllocator().lock(),
                                                         this->_swapchain,
                                                         this->_gpuManager->getLogicalDeviceProxy().lock());
    this->_renderThread->run();
}

void Renderer::destroy() {
    this->_renderThread->stop();

    for (const auto &[stageRef, stage]: this->_renderStages) {
        stage->destroy();
    }

    this->_swapchain->destroy();
}

void Renderer::removeRenderGraph() {
    this->_renderGraph = std::nullopt;
}

void Renderer::setRenderGraph(const RenderGraph &graph) {
    this->_renderGraph = graph;
}

void Renderer::addRenderStage(const RenderStageRef &stageRef, const std::shared_ptr<RenderStage> &stage) {
    this->_renderStages[stageRef] = stage;
}

const std::optional<std::shared_ptr<RenderStage>> Renderer::tryGetRenderStage(const RenderStageRef &stageRef) {
    auto it = this->_renderStages.find(stageRef);

    if (it == this->_renderStages.end()) {
        return std::nullopt;
    }

    return it->second;
}
