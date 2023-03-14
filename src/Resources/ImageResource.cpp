#include "ImageResource.hpp"

#include <stb/stb_image.h>

#include "src/Rendering/RendererAllocator.hpp"

ImageObject *ImageResource::loadImage() {
    ImageData imageData = this->loadImageData(this->_paths[0]);
    ImageObject *image = this->_rendererAllocator->uploadImage(imageData.width, imageData.height,
                                                               imageData.width * imageData.height * 4,
                                                               imageData.data);

    stbi_image_free(imageData.data);

    return image;
}

ImageResource::ImageResource(const std::filesystem::path &path, RendererAllocator *rendererAllocator)
        : ImageBaseResource({path}, rendererAllocator) {
    //
}
