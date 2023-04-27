#include "Renderer.hpp"

#include "src/Engine/EngineError.hpp"
#include "src/Rendering/GpuManager.hpp"
#include "src/Rendering/RenderThread.hpp"
#include "src/Rendering/Swapchain.hpp"
#include "src/Rendering/SwapchainManager.hpp"

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
    this->_swapchain->destroy();
}

void Renderer::removeRenderGraph() {
    this->_renderGraph = std::nullopt;
}

void Renderer::setRenderGraph(const RenderGraph &graph) {
    this->_renderGraph = graph;
}
