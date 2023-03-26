#include "ShadowRenderpass.hpp"

#include "src/Rendering/PhysicalDevice.hpp"
#include "src/Rendering/RenderingDevice.hpp"
#include "src/Rendering/RenderingLayoutsManager.hpp"
#include "src/Rendering/VulkanObjectsAllocator.hpp"
#include "src/Rendering/Builders/RenderpassBuilder.hpp"
#include "src/Rendering/Builders/AttachmentBuilder.hpp"
#include "src/Rendering/Builders/SubpassBuilder.hpp"
#include "src/Rendering/Builders/PipelineBuilder.hpp"
#include "src/Rendering/Builders/FramebufferBuilder.hpp"
#include "src/Rendering/Objects/BufferObject.hpp"
#include "src/Rendering/Objects/ImageViewObject.hpp"
#include "src/Rendering/Types/MeshConstants.hpp"
#include "src/Resources/ResourceManager.hpp"
#include "src/Resources/MeshResource.hpp"
#include "src/Resources/ShaderResource.hpp"

VkFramebuffer ShadowRenderpass::createFramebuffer(const std::shared_ptr<ImageViewObject> &imageView,
                                                  VkExtent2D extent) {
    return FramebufferBuilder(this->_renderingDevice.get(), this->_renderpass)
            .withExtent(extent)
            .addAttachment(imageView->getHandle())
            .build();
}

ShadowRenderpass::ShadowRenderpass(const std::shared_ptr<RenderingDevice> &renderingDevice,
                                   const std::shared_ptr<PhysicalDevice> &physicalDevice,
                                   const std::shared_ptr<RenderingLayoutsManager> &renderingLayoutsManager,
                                   const std::shared_ptr<VulkanObjectsAllocator> &vulkanObjectsAllocator,
                                   const std::shared_ptr<ResourceManager> &resourceManager)
        : RenderpassBase(renderingDevice, {VkClearValue{.depthStencil = {1, 0}},}),
          _physicalDevice(physicalDevice),
          _renderingLayoutsManager(renderingLayoutsManager),
          _vulkanObjectsAllocator(vulkanObjectsAllocator),
          _resourceManager(resourceManager) {
    //
}

void ShadowRenderpass::record(VkCommandBuffer commandBuffer, const ShadowData &shadow,
                              const std::vector<ModelData> &models) {
    VkDeviceSize offset = 0;
    MeshConstants meshConstants;

    vkCmdBindPipeline(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, this->_pipeline);
    for (uint32_t idx = 0; idx < models.size(); idx++) {
        ModelData model = models[idx];

        meshConstants.matrix = shadow.matrix * model.model;
        meshConstants.model = model.model;
        meshConstants.modelRotation = model.modelRotation;
        vkCmdPushConstants(commandBuffer, this->_renderingLayoutsManager->shadowPipelineLayout(),
                           VK_SHADER_STAGE_VERTEX_BIT, 0, sizeof(MeshConstants), &meshConstants);

        VkBuffer vertexBuffer = model.vertices->getHandle();
        vkCmdBindVertexBuffers(commandBuffer, 0, 1, &vertexBuffer, &offset);
        vkCmdBindIndexBuffer(commandBuffer, model.indices->getHandle(), 0, VK_INDEX_TYPE_UINT32);

        vkCmdDrawIndexed(commandBuffer, model.count, 1, 0, 0, idx);
    }
}

void ShadowRenderpass::beginRenderpass(VkCommandBuffer commandBuffer) {
    RenderpassBase::beginRenderpass(commandBuffer);

    const VkViewport viewport = {
            .x = 0,
            .y = 0,
            .width = (float) this->_targetRenderArea.extent.width,
            .height = (float) this->_targetRenderArea.extent.height,
            .minDepth = 0.0f,
            .maxDepth = 1.0f
    };

    vkCmdSetViewport(commandBuffer, 0, 1, &viewport);
    vkCmdSetScissor(commandBuffer, 0, 1, &this->_targetRenderArea);
    vkCmdSetDepthBias(commandBuffer, 1.25f, 0.0f, 1.75f);
}

void ShadowRenderpass::initRenderpass() {
    this->_renderpass = RenderpassBuilder(this->_renderingDevice.get())
            .addAttachment([&](AttachmentBuilder &builder) {
                builder
                        .clear()
                        .withFormat(this->_physicalDevice->getDepthFormat())
                        .withFinalLayout(VK_IMAGE_LAYOUT_DEPTH_STENCIL_READ_ONLY_OPTIMAL);
            })
            .addSubpass([](SubpassBuilder &builder) {
                builder.withDepthAttachment(0, VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL);
            })
            .addSubpassDependency(VK_SUBPASS_EXTERNAL, 0,
                                  VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT,
                                  VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT,
                                  0,
                                  VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT)
            .addSubpassDependency(0, VK_SUBPASS_EXTERNAL,
                                  VK_PIPELINE_STAGE_LATE_FRAGMENT_TESTS_BIT,
                                  VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT,
                                  0,
                                  VK_ACCESS_SHADER_READ_BIT)
            .build();

    std::shared_ptr<ShaderResource> vertexShader = this->_resourceManager->loadShader("shaders/shadow.vert");
    std::shared_ptr<ShaderResource> fragmentShader = this->_resourceManager->loadShader("shaders/shadow.frag");

    this->_pipeline = PipelineBuilder(this->_vulkanObjectsAllocator, this->_renderpass,
                                      this->_renderingLayoutsManager->shadowPipelineLayout())
            .addVertexShader(vertexShader->shader())
            .addFragmentShader(fragmentShader->shader())
            .addBinding(0, sizeof(Vertex), VK_VERTEX_INPUT_RATE_VERTEX)
            .addAttribute(0, 0, offsetof(Vertex, pos), VK_FORMAT_R32G32B32_SFLOAT)
            .withCullMode(VK_CULL_MODE_NONE)
            .withDepthBias()
            .build();

    vertexShader->unload();
    fragmentShader->unload();
}

void ShadowRenderpass::destroyRenderpass() {
    this->_vulkanObjectsAllocator->destroyPipeline(this->_pipeline);

    RenderpassBase::destroyRenderpass();
}
