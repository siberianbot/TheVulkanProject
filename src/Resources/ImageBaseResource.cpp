#include "ImageBaseResource.hpp"

#include <stdexcept>

#define STB_IMAGE_IMPLEMENTATION
#include <stb/stb_image.h>

#include "src/Rendering/Objects/ImageObject.hpp"

ImageBaseResource::ImageBaseResource(const std::vector<std::filesystem::path> &paths,
                                     RendererAllocator *rendererAllocator)
        : Resource(paths),
          _rendererAllocator(rendererAllocator) {
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
        delete this->_image;
        this->_image = nullptr;
    }

    Resource::unload();
}
