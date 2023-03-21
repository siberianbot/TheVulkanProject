#include "CubeImageResource.hpp"

#include <stb/stb_image.h>

#include "src/Rendering/RendererAllocator.hpp"

std::shared_ptr<ImageObject> CubeImageResource::loadImage() {
    std::array<ImageData, 6> imageData;
    std::array<void *, 6> imageDataPtr;
    uint32_t width, height, size;

    for (uint32_t idx = 0; idx < 6; idx++) {
        imageData[idx] = this->loadImageData(this->_paths[idx]);

        width = imageData[idx].width;
        height = imageData[idx].height;
        size = imageData[idx].width * imageData[idx].height * 4;

        imageDataPtr[idx] = imageData[idx].data;
    }

    std::shared_ptr<ImageObject> image = this->_rendererAllocator->uploadCubeImage(width, height, size, imageDataPtr);

    for (uint32_t idx = 0; idx < 6; idx++) {
        stbi_image_free(imageDataPtr[idx]);
    }

    return image;
}

CubeImageResource::CubeImageResource(const std::string &id, const std::array<std::filesystem::path, 6> &paths,
                                     const std::shared_ptr<RendererAllocator> &rendererAllocator)
        : ImageBaseResource(id, std::vector(paths.begin(), paths.end()), rendererAllocator) {
    //
}
