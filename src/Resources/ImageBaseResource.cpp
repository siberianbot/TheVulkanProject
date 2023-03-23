#include "ImageBaseResource.hpp"

#include <stdexcept>

#define STB_IMAGE_IMPLEMENTATION
#include <stb/stb_image.h>

#include "src/Rendering/Objects/ImageObject.hpp"

ImageBaseResource::ImageBaseResource(const std::string &id, const std::vector<std::filesystem::path> &paths,
                                     const std::shared_ptr<RenderingObjectsAllocator> &renderingObjectsAllocator)
        : Resource(id, paths),
          _renderingObjectsAllocator(renderingObjectsAllocator) {
    //
}

ImageBaseResource::ImageData ImageBaseResource::loadImageData(const std::filesystem::path &path) {
    ImageData imageData;

    imageData.data = stbi_load(path.c_str(), &imageData.width, &imageData.height, &imageData.channels, STBI_rgb_alpha);

    if (imageData.data == nullptr) {
        throw std::runtime_error("Failed to load image data");
    }

    return imageData;
}

void ImageBaseResource::load() {
    if (this->_isLoaded) {
        return;
    }

    this->_image = this->loadImage();

    Resource::load();
}

void ImageBaseResource::unload() {
    if (!this->_isLoaded) {
        return;
    }

    if (this->_image != nullptr) {
        this->_image->destroy();
        this->_image = nullptr;
    }

    Resource::unload();
}
