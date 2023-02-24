#include "SkyboxRenderpass.hpp"

#include "src/Engine.hpp"
#include "src/Resources/Mesh.hpp"
#include "src/Resources/Texture.hpp"
#include "src/Rendering/RenderingDevice.hpp"
#include "src/Rendering/RenderingObjectsFactory.hpp"
#include "src/Rendering/Swapchain.hpp"
#include "src/Rendering/RenderpassBuilder.hpp"
#include "src/Rendering/PipelineBuilder.hpp"
#include "src/Rendering/FramebuffersBuilder.hpp"
#include "src/Rendering/Objects/BufferObject.hpp"
#include "src/Rendering/Objects/ImageObject.hpp"
#include "src/Rendering/Objects/ImageViewObject.hpp"
#include "src/Rendering/Objects/DescriptorSetObject.hpp"
#include "src/Rendering/Objects/RenderingLayoutObject.hpp"

#define GLM_FORCE_RADIANS
#define GLM_FORCE_DEPTH_ZERO_TO_ONE
#define GLM_ENABLE_EXPERIMENTAL

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

static constexpr const char SKYBOX_TEXTURE_PATH[] = "data/textures/skybox.png";
static constexpr std::array<Vertex, 36> SKYBOX_TRIANGLES = {
        // up
        Vertex{.pos = {-1, 1, -1}, .color = {1, 1, 1}, .texCoord = {0.25, 0}},
        Vertex{.pos = {1, 1, -1}, .color = {1, 1, 1}, .texCoord = {0.5, 0}},
        Vertex{.pos = {1, 1, 1}, .color = {1, 1, 1}, .texCoord = {0.5, 1.0 / 3}},
        Vertex{.pos = {1, 1, 1}, .color = {1, 1, 1}, .texCoord = {0.5, 1.0 / 3}},
        Vertex{.pos = {-1, 1, 1}, .color = {1, 1, 1}, .texCoord = {0.25, 1.0 / 3}},
        Vertex{.pos = {-1, 1, -1}, .color = {1, 1, 1}, .texCoord = {0.25, 0}},

        // front
        Vertex{.pos = {-1, 1, 1}, .color = {1, 1, 1}, .texCoord = {0.25, 1.0 / 3}},
        Vertex{.pos = {1, 1, 1}, .color = {1, 1, 1}, .texCoord = {0.5, 1.0 / 3}},
        Vertex{.pos = {1, -1, 1}, .color = {1, 1, 1}, .texCoord = {0.5, 2.0 / 3}},
        Vertex{.pos = {1, -1, 1}, .color = {1, 1, 1}, .texCoord = {0.5, 2.0 / 3}},
        Vertex{.pos = {-1, -1, 1}, .color = {1, 1, 1}, .texCoord = {0.25, 2.0 / 3}},
        Vertex{.pos = {-1, 1, 1}, .color = {1, 1, 1}, .texCoord = {0.25, 1.0 / 3}},

        // left
        Vertex{.pos = {-1, 1, -1}, .color = {1, 1, 1}, .texCoord = {0, 1.0 / 3}},
        Vertex{.pos = {-1, 1, 1}, .color = {1, 1, 1}, .texCoord = {0.25, 1.0 / 3}},
        Vertex{.pos = {-1, -1, 1}, .color = {1, 1, 1}, .texCoord = {0.25, 2.0 / 3}},
        Vertex{.pos = {-1, -1, 1}, .color = {1, 1, 1}, .texCoord = {0.25, 2.0 / 3}},
        Vertex{.pos = {-1, -1, -1}, .color = {1, 1, 1}, .texCoord = {0, 2.0 / 3}},
        Vertex{.pos = {-1, 1, -1}, .color = {1, 1, 1}, .texCoord = {0, 1.0 / 3}},

        // right
        Vertex{.pos = {1, 1, 1}, .color = {1, 1, 1}, .texCoord = {0.5, 1.0 / 3}},
        Vertex{.pos = {1, 1, -1}, .color = {1, 1, 1}, .texCoord = {0.75, 1.0 / 3}},
        Vertex{.pos = {1, -1, -1}, .color = {1, 1, 1}, .texCoord = {0.75, 2.0 / 3}},
        Vertex{.pos = {1, -1, -1}, .color = {1, 1, 1}, .texCoord = {0.75, 2.0 / 3}},
        Vertex{.pos = {1, -1, 1}, .color = {1, 1, 1}, .texCoord = {0.5, 2.0 / 3}},
        Vertex{.pos = {1, 1, 1}, .color = {1, 1, 1}, .texCoord = {0.5, 1.0 / 3}},

        // back
        Vertex{.pos = {1, 1, -1}, .color = {1, 1, 1}, .texCoord = {0.75, 1.0 / 3}},
        Vertex{.pos = {-1, 1, -1}, .color = {1, 1, 1}, .texCoord = {1, 1.0 / 3}},
        Vertex{.pos = {-1, -1, -1}, .color = {1, 1, 1}, .texCoord = {1, 2.0 / 3}},
        Vertex{.pos = {-1, -1, -1}, .color = {1, 1, 1}, .texCoord = {1, 2.0 / 3}},
        Vertex{.pos = {1, -1, -1}, .color = {1, 1, 1}, .texCoord = {0.75, 2.0 / 3}},
        Vertex{.pos = {1, 1, -1}, .color = {1, 1, 1}, .texCoord = {0.75, 1.0 / 3}},

        // down
        Vertex{.pos = {-1, -1, 1}, .color = {1, 1, 1}, .texCoord = {0.25, 2.0 / 3}},
        Vertex{.pos = {1, -1, 1}, .color = {1, 1, 1}, .texCoord = {0.5, 2.0 / 3}},
        Vertex{.pos = {1, -1, -1}, .color = {1, 1, 1}, .texCoord = {0.5, 1}},
        Vertex{.pos = {1, -1, -1}, .color = {1, 1, 1}, .texCoord = {0.5, 1}},
        Vertex{.pos = {-1, -1, -1}, .color = {1, 1, 1}, .texCoord = {0.25, 1}},
        Vertex{.pos = {-1, -1, 1}, .color = {1, 1, 1}, .texCoord = {0.25, 2.0 / 3}},
};

SkyboxRenderpass::SkyboxRenderpass(RenderingDevice *renderingDevice, Swapchain *swapchain,
                                   RenderingObjectsFactory *renderingObjectsFactory,
                                   RenderingResourcesManager *renderingResourcesManager, Engine *engine)
        : RenderpassBase(renderingDevice, swapchain),
          _engine(engine),
          _renderingResourcesManager(renderingResourcesManager) {
    this->_meshResource = this->_renderingResourcesManager->loadMesh(SKYBOX_TRIANGLES.size(),
                                                                     SKYBOX_TRIANGLES.data());

    Texture texture = Texture::fromFile(SKYBOX_TEXTURE_PATH);
    this->_textureResource = this->_renderingResourcesManager->loadTexture(&texture);
    this->_textureView = renderingObjectsFactory->createImageViewObject(this->_textureResource.texture,
                                                                        VK_IMAGE_ASPECT_COLOR_BIT);

    this->_textureSampler = this->_renderingDevice->createSampler();
    this->_renderingLayoutObject = renderingObjectsFactory->createRenderingLayoutObject();
    this->_descriptorSet = this->_renderingLayoutObject->createMeshDataDescriptor(this->_textureSampler,
                                                                                  this->_textureView->getHandle());
}

SkyboxRenderpass::~SkyboxRenderpass() {
    delete this->_descriptorSet;
    delete this->_renderingLayoutObject;
    this->_renderingDevice->destroySampler(this->_textureSampler);

    delete this->_textureView;
    this->_renderingResourcesManager->freeTexture(this->_textureResource);
    this->_renderingResourcesManager->freeMesh(this->_meshResource);
}

void SkyboxRenderpass::recordCommands(VkCommandBuffer commandBuffer, VkRect2D renderArea,
                                      uint32_t frameIdx, uint32_t imageIdx) {
    VkExtent2D extent = this->_swapchain->getSwapchainExtent();
    VkPipelineLayout pipelineLayout = this->_renderingLayoutObject->getPipelineLayout();

    SceneData *sceneData = this->_renderingLayoutObject->getSceneData(frameIdx);
    sceneData->view = glm::lookAt(glm::vec3(0), this->_engine->camera().forward(), glm::vec3(0, 1, 0));
    sceneData->projection = glm::perspective(glm::radians(45.0f), extent.width / (float) extent.height, 0.1f, 100.0f);
    sceneData->projection[1][1] *= -1;

    VkDescriptorSet sceneDataDescriptorSet = this->_renderingLayoutObject->getSceneDataDescriptorSetObject()
            ->getDescriptorSet(frameIdx);

    const VkRenderPassBeginInfo renderPassBeginInfo = {
            .sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO,
            .pNext = nullptr,
            .renderPass = this->_renderpass,
            .framebuffer = this->_framebuffers[imageIdx],
            .renderArea = renderArea,
            .clearValueCount = 0,
            .pClearValues = nullptr
    };

    vkCmdBeginRenderPass(commandBuffer, &renderPassBeginInfo, VK_SUBPASS_CONTENTS_INLINE);

    vkCmdBindPipeline(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, this->_pipeline);

    VkViewport viewport = {
            .x = 0,
            .y = 0,
            .width = (float) renderArea.extent.width,
            .height = (float) renderArea.extent.height,
            .minDepth = 0.0f,
            .maxDepth = 1.0f
    };
    vkCmdSetViewport(commandBuffer, 0, 1, &viewport);

    VkRect2D scissor = {
            .offset = {0, 0},
            .extent = renderArea.extent
    };
    vkCmdSetScissor(commandBuffer, 0, 1, &scissor);

    VkDeviceSize offset = 0;
    VkDescriptorSet descriptorSet = this->_descriptorSet->getDescriptorSet(frameIdx);

    MeshConstants constants = {
            .model = glm::mat4(1)
    };
    vkCmdPushConstants(commandBuffer, pipelineLayout, VK_SHADER_STAGE_VERTEX_BIT, 0, sizeof(MeshConstants),
                       &constants);

    vkCmdBindDescriptorSets(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, pipelineLayout,
                            0, 1, &sceneDataDescriptorSet, 0, nullptr);

    vkCmdBindDescriptorSets(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, pipelineLayout,
                            1, 1, &descriptorSet, 0, nullptr);

    VkBuffer vertexBuffer = this->_meshResource.vertices->getHandle();
    vkCmdBindVertexBuffers(commandBuffer, 0, 1, &vertexBuffer, &offset);

    vkCmdDraw(commandBuffer, SKYBOX_TRIANGLES.size(), 1, 0, 0);

    vkCmdEndRenderPass(commandBuffer);
}

void SkyboxRenderpass::initRenderpass() {
    this->_renderpass = RenderpassBuilder(this->_renderingDevice)
            .noDepthAttachment()
            .load()
            .build();

    this->_pipeline = PipelineBuilder(this->_renderingDevice, this->_renderpass,
                                      this->_renderingLayoutObject->getPipelineLayout())
            .addVertexShader(DEFAULT_VERTEX_SHADER)
            .addFragmentShader(DEFAULT_FRAGMENT_SHADER)
            .addBinding(0, sizeof(Vertex), VK_VERTEX_INPUT_RATE_VERTEX)
            .addAttribute(0, 0, offsetof(Vertex, pos), VK_FORMAT_R32G32B32_SFLOAT)
            .addAttribute(0, 1, offsetof(Vertex, color), VK_FORMAT_R32G32B32_SFLOAT)
            .addAttribute(0, 2, offsetof(Vertex, texCoord), VK_FORMAT_R32G32_SFLOAT)
            .build();
}

void SkyboxRenderpass::destroyRenderpass() {
    vkDestroyPipeline(this->_renderingDevice->getHandle(), this->_pipeline, nullptr);

    RenderpassBase::destroyRenderpass();
}

void SkyboxRenderpass::createFramebuffers() {
    this->_framebuffers = FramebuffersBuilder(this->_renderingDevice, this->_swapchain, this->_renderpass)
            .build();
}
