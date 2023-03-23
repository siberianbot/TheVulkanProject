#include "RenderpassBase.hpp"

#include "src/Rendering/RenderingDevice.hpp"

//RenderpassBase::RenderpassBase(const std::shared_ptr<RenderingDevice> &renderingDevice)
//        : _renderingDevice(renderingDevice) {
//    //
//}
//
//void RenderpassBase::destroyRenderpass() {
//    this->_renderingDevice->destroyRenderpass(this->_renderpass);
//}
//
//void RenderpassBase::destroyFramebuffers() {
//    for (const VkFramebuffer &framebuffer: this->_framebuffers) {
//        this->_renderingDevice->destroyFramebuffer(framebuffer);
//    }
//}
//
//void RenderpassBase::setTargetImageViews(const std::vector<std::shared_ptr<ImageViewObject>> &targetImageViews) {
//
//}
