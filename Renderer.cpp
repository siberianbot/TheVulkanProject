#include "Renderer.hpp"

#include <set>
#include <iostream>
#include <array>

#include "Constants.hpp"
#include "Engine.hpp"
#include "Rendering/Renderpasses/ClearRenderpass.hpp"
#include "Rendering/Renderpasses/FinalRenderpass.hpp"
#include "VulkanCommon.hpp"

#define STB_IMAGE_IMPLEMENTATION

#include <stb/stb_image.h>

std::vector<const char *> vkRequiredExtensions() {
    uint32_t count;
    const char **extensions = glfwGetRequiredInstanceExtensions(&count);

    std::vector<const char *> result(extensions, extensions + count);
    result.push_back(VK_EXT_DEBUG_UTILS_EXTENSION_NAME);

    return result;
}

static VKAPI_ATTR VkBool32 VKAPI_CALL vkDebugCallback(VkDebugUtilsMessageSeverityFlagBitsEXT messageSeverity,
                                                      VkDebugUtilsMessageTypeFlagsEXT messageType,
                                                      const VkDebugUtilsMessengerCallbackDataEXT *pCallbackData,
                                                      void *pUserData) {
    const char *type;
    switch (messageType) {
        case VkDebugUtilsMessageTypeFlagBitsEXT::VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT:
            type = "vk validation";
            break;

        case VkDebugUtilsMessageTypeFlagBitsEXT::VK_DEBUG_UTILS_MESSAGE_TYPE_PERFORMANCE_BIT_EXT:
            type = "vk performance";
            break;

        case VkDebugUtilsMessageTypeFlagBitsEXT::VK_DEBUG_UTILS_MESSAGE_TYPE_DEVICE_ADDRESS_BINDING_BIT_EXT:
            type = "vk binding";
            break;

        case VkDebugUtilsMessageTypeFlagBitsEXT::VK_DEBUG_UTILS_MESSAGE_TYPE_GENERAL_BIT_EXT:
            type = "vk general";
            break;

        default:
            throw std::runtime_error("not supported");
    }

    if (messageSeverity == VkDebugUtilsMessageSeverityFlagBitsEXT::VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT) {
        std::cerr << type << ": " << pCallbackData->pMessage << std::endl;
    } else {
        std::cout << type << ": " << pCallbackData->pMessage << std::endl;
    }

    return VK_FALSE;
}

Renderer::Renderer(Engine *engine) : engine(engine) {
    //
}

void Renderer::init() {
    initInstance();
    initSurface(this->engine->window());

    this->_physicalDevice = VulkanPhysicalDevice::selectSuitable(this->instance, this->surface);
    this->_renderingDevice = this->_physicalDevice->createRenderingDevice();
    this->_renderingObjectsFactory = new RenderingObjectsFactory(this->_renderingDevice);
    this->_commandExecutor = new VulkanCommandExecutor(this->_renderingDevice);
    this->_swapchain = new Swapchain(this->_renderingDevice, this->_renderingObjectsFactory);

    for (uint32_t frameIdx = 0; frameIdx < MAX_INFLIGHT_FRAMES; frameIdx++) {
        this->_syncObjectsGroups[frameIdx] = new SyncObjectsGroup{
                .fence =  this->_renderingObjectsFactory->createFenceObject(true),
                .imageAvailableSemaphore = this->_renderingObjectsFactory->createSemaphoreObject(),
                .renderFinishedSemaphore = this->_renderingObjectsFactory->createSemaphoreObject()
        };
    }

    initUniformBuffers();
    initTextureSampler();
    initDescriptors();
    initLayouts();

    this->_swapchain->create();

    this->_renderpasses.push_back(new ClearRenderpass(this->_renderingDevice, this->_swapchain));
    this->_sceneRenderpass = new SceneRenderpass(this->_renderingDevice, this->_swapchain, this->pipelineLayout);
    this->_renderpasses.push_back(this->_sceneRenderpass);
    this->_renderpasses.push_back(new FinalRenderpass(this->_renderingDevice, this->_swapchain));

    for (RenderpassBase *renderpass: this->_renderpasses) {
        renderpass->initRenderpass();
        renderpass->createFramebuffers();
    }
}

void Renderer::cleanup() {
    this->_renderingDevice->waitIdle();

    for (RenderpassBase *renderpass: this->_renderpasses) {
        renderpass->destroyFramebuffers();
    }

    this->_swapchain->destroy();

    this->_sceneRenderpass = nullptr;
    for (RenderpassBase *renderpass: this->_renderpasses) {
        renderpass->destroyRenderpass();
        delete renderpass;
    }

    vkDestroyPipelineLayout(this->_renderingDevice->getHandle(), this->pipelineLayout, nullptr);

    vkDestroyDescriptorSetLayout(this->_renderingDevice->getHandle(), this->descriptorSetLayout, nullptr);
    vkDestroyDescriptorPool(this->_renderingDevice->getHandle(), this->descriptorPool, nullptr);

    vkDestroySampler(this->_renderingDevice->getHandle(), this->textureSampler, nullptr);

    for (uint32_t frameIdx = 0; frameIdx < MAX_INFLIGHT_FRAMES; frameIdx++) {
        delete this->uniformBuffers[frameIdx];

        delete this->_syncObjectsGroups[frameIdx];
    }

    delete this->_swapchain;
    delete this->_commandExecutor;
    delete this->_renderingObjectsFactory;
    delete this->_renderingDevice;
    delete this->_physicalDevice;

    vkDestroySurfaceKHR(this->instance, this->surface, nullptr);
    vkDestroyInstance(this->instance, nullptr);
}

void Renderer::requestResize(uint32_t width, uint32_t height) {
    this->resizeRequested = true;
}

void Renderer::initInstance() {
    auto extensions = vkRequiredExtensions();
    uint32_t version = VK_MAKE_VERSION(0, 1, 0);

    VkApplicationInfo appInfo = {
            .sType = VkStructureType::VK_STRUCTURE_TYPE_APPLICATION_INFO,
            .pNext = nullptr,
            .pApplicationName = NAME,
            .applicationVersion = version,
            .pEngineName = NAME,
            .engineVersion = version,
            .apiVersion = VK_API_VERSION_1_1,
    };

    VkDebugUtilsMessengerCreateInfoEXT debugUtilsMessengerCreateInfoExt = {
            .sType = VkStructureType::VK_STRUCTURE_TYPE_DEBUG_UTILS_MESSENGER_CREATE_INFO_EXT,
            .pNext = nullptr,
            .flags = 0,
            .messageSeverity = VkDebugUtilsMessageSeverityFlagBitsEXT::VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT |
                               VkDebugUtilsMessageSeverityFlagBitsEXT::VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT |
                               VkDebugUtilsMessageSeverityFlagBitsEXT::VK_DEBUG_UTILS_MESSAGE_SEVERITY_INFO_BIT_EXT |
                               VkDebugUtilsMessageSeverityFlagBitsEXT::VK_DEBUG_UTILS_MESSAGE_SEVERITY_VERBOSE_BIT_EXT,
            .messageType = VkDebugUtilsMessageTypeFlagBitsEXT::VK_DEBUG_UTILS_MESSAGE_TYPE_GENERAL_BIT_EXT |
                           VkDebugUtilsMessageTypeFlagBitsEXT::VK_DEBUG_UTILS_MESSAGE_TYPE_DEVICE_ADDRESS_BINDING_BIT_EXT |
                           VkDebugUtilsMessageTypeFlagBitsEXT::VK_DEBUG_UTILS_MESSAGE_TYPE_PERFORMANCE_BIT_EXT |
                           VkDebugUtilsMessageTypeFlagBitsEXT::VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT,
            .pfnUserCallback = vkDebugCallback,
            .pUserData = this
    };

    VkInstanceCreateInfo instanceCreateInfo = {
            .sType = VkStructureType::VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO,
            .pNext = &debugUtilsMessengerCreateInfoExt,
            .flags = 0,
            .pApplicationInfo = &appInfo,
            .enabledLayerCount = static_cast<uint32_t>(VK_VALIDATION_LAYERS.size()),
            .ppEnabledLayerNames = VK_VALIDATION_LAYERS.data(),
            .enabledExtensionCount = static_cast<uint32_t>(extensions.size()),
            .ppEnabledExtensionNames = extensions.data()
    };

    vkEnsure(vkCreateInstance(&instanceCreateInfo, nullptr, &this->instance));
}

void Renderer::initSurface(GLFWwindow *window) {
    vkEnsure(glfwCreateWindowSurface(this->instance, window, nullptr, &this->surface));
}

void Renderer::initUniformBuffers() {
    VkDeviceSize uboSize = sizeof(UniformBufferObject);

    for (size_t idx = 0; idx < VK_MAX_INFLIGHT_FRAMES; idx++) {
        this->uniformBuffers[idx] = this->_renderingObjectsFactory
                ->createBufferObject(uboSize, VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT,
                                     VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT);
    }
}

void Renderer::initTextureSampler() {
    VkSamplerCreateInfo samplerCreateInfo = {
            .sType = VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO,
            .pNext = nullptr,
            .flags = 0,
            .magFilter = VK_FILTER_LINEAR,
            .minFilter = VK_FILTER_LINEAR,
            .mipmapMode = VkSamplerMipmapMode::VK_SAMPLER_MIPMAP_MODE_LINEAR,
            .addressModeU = VK_SAMPLER_ADDRESS_MODE_REPEAT,
            .addressModeV = VK_SAMPLER_ADDRESS_MODE_REPEAT,
            .addressModeW = VK_SAMPLER_ADDRESS_MODE_REPEAT,
            .mipLodBias = 0,
            .anisotropyEnable = VK_TRUE,
            .maxAnisotropy = this->_physicalDevice->getMaxSamplerAnisotropy(),
            .compareEnable = VK_FALSE,
            .compareOp = VK_COMPARE_OP_ALWAYS,
            .minLod = 0,
            .maxLod = 1,
            .borderColor = VK_BORDER_COLOR_FLOAT_OPAQUE_BLACK,
            .unnormalizedCoordinates = VK_FALSE
    };

    vkEnsure(vkCreateSampler(this->_renderingDevice->getHandle(), &samplerCreateInfo, nullptr, &this->textureSampler));
}

void Renderer::initDescriptors() {
    std::array<VkDescriptorPoolSize, 2> descriptorPoolSizes = {
            VkDescriptorPoolSize{
                    .type = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER,
                    .descriptorCount = VK_MAX_INFLIGHT_FRAMES
            },
            VkDescriptorPoolSize{
                    .type = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER,
                    .descriptorCount = VK_MAX_INFLIGHT_FRAMES
            }
    };

    VkDescriptorPoolCreateInfo descriptorPoolCreateInfo = {
            .sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO,
            .pNext = nullptr,
            .flags = VK_DESCRIPTOR_POOL_CREATE_FREE_DESCRIPTOR_SET_BIT,
            .maxSets = 4 * VK_MAX_INFLIGHT_FRAMES,
            .poolSizeCount = static_cast<uint32_t>(descriptorPoolSizes.size()),
            .pPoolSizes = descriptorPoolSizes.data()
    };

    vkEnsure(vkCreateDescriptorPool(this->_renderingDevice->getHandle(), &descriptorPoolCreateInfo, nullptr,
                                    &this->descriptorPool));
}

void Renderer::initLayouts() {
    std::array<VkDescriptorSetLayoutBinding, 2> bindings = {
            VkDescriptorSetLayoutBinding{
                    .binding = 0,
                    .descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER,
                    .descriptorCount = 1,
                    .stageFlags = VK_SHADER_STAGE_VERTEX_BIT,
                    .pImmutableSamplers = nullptr
            },
            VkDescriptorSetLayoutBinding{
                    .binding = 1,
                    .descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER,
                    .descriptorCount = 1,
                    .stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT,
                    .pImmutableSamplers = nullptr
            }
    };

    VkDescriptorSetLayoutCreateInfo descriptorSetLayoutCreateInfo = {
            .sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO,
            .pNext = nullptr,
            .flags = 0,
            .bindingCount = static_cast<uint32_t>(bindings.size()),
            .pBindings = bindings.data()
    };

    vkEnsure(vkCreateDescriptorSetLayout(this->_renderingDevice->getHandle(), &descriptorSetLayoutCreateInfo, nullptr,
                                         &this->descriptorSetLayout));

    VkPushConstantRange constantRange = {
            .stageFlags = VK_SHADER_STAGE_VERTEX_BIT,
            .offset = 0,
            .size = sizeof(Constants)
    };

    VkPipelineLayoutCreateInfo pipelineLayoutInfo = {
            .sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO,
            .pNext = nullptr,
            .flags = 0,
            .setLayoutCount = 1,
            .pSetLayouts = &this->descriptorSetLayout,
            .pushConstantRangeCount = 1,
            .pPushConstantRanges = &constantRange
    };

    vkEnsure(vkCreatePipelineLayout(this->_renderingDevice->getHandle(), &pipelineLayoutInfo, nullptr,
                                    &this->pipelineLayout));
}

void Renderer::handleResize() {
    this->_renderingDevice->waitIdle();

    for (RenderpassBase *renderpass: this->_renderpasses) {
        renderpass->destroyFramebuffers();
    }

    this->_swapchain->create();

    for (RenderpassBase *renderpass: this->_renderpasses) {
        renderpass->createFramebuffers();
    }
}

void Renderer::render() {
    SyncObjectsGroup *currentSyncObjects = this->_syncObjectsGroups[_currentFrameIdx];

    currentSyncObjects->fence->wait(UINT64_MAX);
    currentSyncObjects->fence->reset();

    uint32_t imageIdx;
    VkResult result = vkAcquireNextImageKHR(this->_renderingDevice->getHandle(), this->_swapchain->getHandle(),
                                            UINT64_MAX, currentSyncObjects->imageAvailableSemaphore->getHandle(),
                                            VK_NULL_HANDLE, &imageIdx);

    if (result == VK_ERROR_OUT_OF_DATE_KHR) {
        handleResize();
        return;
    } else if (result != VkResult::VK_SUCCESS && result != VkResult::VK_SUBOPTIMAL_KHR) {
        throw std::runtime_error("Vulkan acquire next image failure");
    }

    VkExtent2D extent = this->_swapchain->getSwapchainExtent();

    ubo.view = this->engine->camera().view();
    ubo.proj = glm::perspective(glm::radians(45.0f), extent.width / (float) extent.height, 0.1f, 10.0f);
    ubo.proj[1][1] *= -1;
    memcpy(this->uniformBuffers[_currentFrameIdx]->map(), &ubo, sizeof(ubo));

    this->_commandExecutor->beginMainExecution(_currentFrameIdx, [this, &extent, &imageIdx](VkCommandBuffer cmdBuffer) {
                VkRect2D renderArea = {
                        .offset = {0, 0},
                        .extent = extent
                };

                for (RenderpassBase *renderpass: this->_renderpasses) {
                    renderpass->recordCommands(cmdBuffer, renderArea, _currentFrameIdx, imageIdx);
                }
            })
            .withFence(currentSyncObjects->fence)
            .withWaitSemaphore(currentSyncObjects->imageAvailableSemaphore)
            .withSignalSemaphore(currentSyncObjects->renderFinishedSemaphore)
            .withWaitDstStageMask(VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT)
            .submit(false);

    VkSwapchainKHR swapchain = this->_swapchain->getHandle();
    VkSemaphore renderFinishedSemaphore = currentSyncObjects->renderFinishedSemaphore->getHandle();

    VkPresentInfoKHR presentInfo = {
            .sType = VkStructureType::VK_STRUCTURE_TYPE_PRESENT_INFO_KHR,
            .pNext = nullptr,
            .waitSemaphoreCount = 1,
            .pWaitSemaphores = &renderFinishedSemaphore,
            .swapchainCount = 1,
            .pSwapchains = &swapchain,
            .pImageIndices = &imageIdx,
            .pResults = nullptr
    };

    result = vkQueuePresentKHR(this->_renderingDevice->getPresentQueue(), &presentInfo);

    if (result == VkResult::VK_ERROR_OUT_OF_DATE_KHR || result == VkResult::VK_SUBOPTIMAL_KHR || resizeRequested) {
        resizeRequested = false;
        handleResize();
    } else if (result != VkResult::VK_SUCCESS) {
        throw std::runtime_error("Vulkan present failure");
    }

    _currentFrameIdx = (_currentFrameIdx + 1) % MAX_INFLIGHT_FRAMES;
}

BufferObject *Renderer::uploadVertices(const std::vector<Vertex> &vertices) {
    // TODO: use staging buffer to restrict usage of GPU memory by CPU

    VkDeviceSize size = sizeof(vertices[0]) * vertices.size();

    BufferObject *buffer = this->_renderingObjectsFactory->createBufferObject(size,
                                                                              VK_BUFFER_USAGE_TRANSFER_SRC_BIT |
                                                                              VK_BUFFER_USAGE_VERTEX_BUFFER_BIT,
                                                                              VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT |
                                                                              VK_MEMORY_PROPERTY_HOST_COHERENT_BIT |
                                                                              VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);

    memcpy(buffer->map(), vertices.data(), size);
    buffer->unmap();

    return buffer;
}

BufferObject *Renderer::uploadIndices(const std::vector<uint32_t> &indices) {
    // TODO: use staging buffer to restrict usage of GPU memory by CPU

    VkDeviceSize size = sizeof(indices[0]) * indices.size();

    BufferObject *buffer = this->_renderingObjectsFactory->createBufferObject(size,
                                                                              VK_BUFFER_USAGE_TRANSFER_SRC_BIT |
                                                                              VK_BUFFER_USAGE_INDEX_BUFFER_BIT,
                                                                              VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT |
                                                                              VK_MEMORY_PROPERTY_HOST_COHERENT_BIT |
                                                                              VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);

    memcpy(buffer->map(), indices.data(), size);
    buffer->unmap();

    return buffer;
}

ImageObject *Renderer::uploadTexture(const std::string &texturePath) {
    int width, height, channels;
    stbi_uc *pixels = stbi_load(texturePath.c_str(), &width, &height, &channels, STBI_rgb_alpha);

    VkDeviceSize imageSize = width * height * 4;

    BufferObject *stagingBuffer = this->_renderingObjectsFactory->createBufferObject(imageSize,
                                                                                     VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
                                                                                     VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT |
                                                                                     VK_MEMORY_PROPERTY_HOST_COHERENT_BIT);

    memcpy(stagingBuffer->map(), pixels, static_cast<size_t>(imageSize));
    stagingBuffer->unmap();
    stbi_image_free(pixels);

    ImageObject *image = this->_renderingObjectsFactory->createImageObject(width, height, VK_FORMAT_R8G8B8A8_SRGB,
                                                                           VK_IMAGE_USAGE_TRANSFER_SRC_BIT |
                                                                           VK_IMAGE_USAGE_TRANSFER_DST_BIT |
                                                                           VK_IMAGE_USAGE_SAMPLED_BIT,
                                                                           VK_SAMPLE_COUNT_1_BIT,
                                                                           VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT,
                                                                           VK_IMAGE_ASPECT_COLOR_BIT);

    this->_commandExecutor->beginOneTimeExecution(
                    [&image, &width, &height, &stagingBuffer](VkCommandBuffer cmdBuffer) {
                        VkImageMemoryBarrier imageMemoryBarrier = {
                                .sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER,
                                .pNext = nullptr,
                                .srcAccessMask = 0,
                                .dstAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT,
                                .oldLayout = VK_IMAGE_LAYOUT_UNDEFINED,
                                .newLayout = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
                                .srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED,
                                .dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED,
                                .image = image->getImageHandle(),
                                .subresourceRange = {
                                        .aspectMask = VK_IMAGE_ASPECT_COLOR_BIT,
                                        .baseMipLevel = 0,
                                        .levelCount = 1,
                                        .baseArrayLayer = 0,
                                        .layerCount = 1
                                }
                        };
                        vkCmdPipelineBarrier(cmdBuffer, VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT,
                                             VK_PIPELINE_STAGE_TRANSFER_BIT,
                                             0, 0, nullptr, 0, nullptr, 1, &imageMemoryBarrier);

                        VkBufferImageCopy bufferImageCopy = {
                                .bufferOffset = 0,
                                .bufferRowLength = 0,
                                .bufferImageHeight = 0,
                                .imageSubresource = {
                                        .aspectMask = VK_IMAGE_ASPECT_COLOR_BIT,
                                        .mipLevel = 0,
                                        .baseArrayLayer = 0,
                                        .layerCount = 1
                                },
                                .imageOffset = {0, 0, 0},
                                .imageExtent = {(uint32_t) width, (uint32_t) height, 1}
                        };
                        vkCmdCopyBufferToImage(cmdBuffer, stagingBuffer->getHandle(), image->getImageHandle(),
                                               VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, 1,
                                               &bufferImageCopy);

                        VkImageMemoryBarrier anotherImageMemoryBarrier = {
                                .sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER,
                                .pNext = nullptr,
                                .srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT,
                                .dstAccessMask = VK_ACCESS_SHADER_READ_BIT,
                                .oldLayout = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
                                .newLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL,
                                .srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED,
                                .dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED,
                                .image = image->getImageHandle(),
                                .subresourceRange = {
                                        .aspectMask = VK_IMAGE_ASPECT_COLOR_BIT,
                                        .baseMipLevel = 0,
                                        .levelCount = 1,
                                        .baseArrayLayer = 0,
                                        .layerCount = 1
                                }
                        };
                        vkCmdPipelineBarrier(cmdBuffer, VK_PIPELINE_STAGE_TRANSFER_BIT,
                                             VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT,
                                             0, 0, nullptr, 0, nullptr, 1, &anotherImageMemoryBarrier);

                    })
            .submit(true);

    delete stagingBuffer;

    return image;
}

std::array<VkDescriptorSet, VK_MAX_INFLIGHT_FRAMES> Renderer::initDescriptorSets(VkImageView textureImageView) {
    std::vector<VkDescriptorSetLayout> layouts(VK_MAX_INFLIGHT_FRAMES, this->descriptorSetLayout);
    std::array<VkDescriptorSet, VK_MAX_INFLIGHT_FRAMES> descriptorSets = {};

    VkDescriptorSetAllocateInfo descriptorSetAllocateInfo = {
            .sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO,
            .pNext = nullptr,
            .descriptorPool = this->descriptorPool,
            .descriptorSetCount = static_cast<uint32_t>(descriptorSets.size()),
            .pSetLayouts = layouts.data()
    };

    vkEnsure(vkAllocateDescriptorSets(this->_renderingDevice->getHandle(), &descriptorSetAllocateInfo,
                                      descriptorSets.data()));

    for (uint32_t idx = 0; idx < VK_MAX_INFLIGHT_FRAMES; idx++) {
        VkDescriptorBufferInfo bufferInfo = {
                .buffer = this->uniformBuffers[idx]->getHandle(),
                .offset = 0,
                .range = sizeof(UniformBufferObject)
        };

        VkDescriptorImageInfo imageInfo = {
                .sampler = this->textureSampler,
                .imageView = textureImageView,
                .imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL
        };

        std::array<VkWriteDescriptorSet, 2> writes = {
                VkWriteDescriptorSet{
                        .sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET,
                        .pNext = nullptr,
                        .dstSet = descriptorSets[idx],
                        .dstBinding = 0,
                        .dstArrayElement = 0,
                        .descriptorCount = 1,
                        .descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER,
                        .pImageInfo = nullptr,
                        .pBufferInfo = &bufferInfo,
                        .pTexelBufferView = nullptr
                },
                VkWriteDescriptorSet{
                        .sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET,
                        .pNext = nullptr,
                        .dstSet = descriptorSets[idx],
                        .dstBinding = 1,
                        .dstArrayElement = 0,
                        .descriptorCount = 1,
                        .descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER,
                        .pImageInfo = &imageInfo,
                        .pBufferInfo = nullptr,
                        .pTexelBufferView = nullptr
                }
        };

        vkUpdateDescriptorSets(this->_renderingDevice->getHandle(), static_cast<uint32_t>(writes.size()), writes.data(),
                               0, nullptr);
    }

    return descriptorSets;
}

BoundMeshInfo *Renderer::uploadMesh(const Mesh &mesh, const Texture &texture) {
    ImageObject *textureData = uploadTexture(texture.path);

    auto boundMeshInfo = new BoundMeshInfo{
            .vertexBuffer = uploadVertices(mesh.vertices),
            .indexBuffer = uploadIndices(mesh.indices),
            .texture = textureData,
            .model = glm::translate(glm::mat4(1.0f), glm::vec3(0.0f, -0.5f, 0.0f)),
            .indicesCount = static_cast<uint32_t>(mesh.indices.size()),
            .descriptorSets = initDescriptorSets(textureData->getImageViewHandle())
    };

    this->_sceneRenderpass->addMesh(boundMeshInfo);

    return boundMeshInfo;
}

void Renderer::freeMesh(BoundMeshInfo *meshInfo) {
    vkEnsure(vkDeviceWaitIdle(this->_renderingDevice->getHandle()));

    vkFreeDescriptorSets(this->_renderingDevice->getHandle(), this->descriptorPool,
                         static_cast<uint32_t>(meshInfo->descriptorSets.size()),
                         meshInfo->descriptorSets.data());

    delete meshInfo->texture;
    delete meshInfo->vertexBuffer;
    delete meshInfo->indexBuffer;

    this->_sceneRenderpass->removeMesh(meshInfo);
}
