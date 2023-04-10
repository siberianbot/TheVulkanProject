#include "TextureRenderingData.hpp"

#include "src/Rendering/Objects/ImageObject.hpp"
#include "src/Rendering/Objects/ImageViewObject.hpp"

void TextureRenderingData::free() {
    this->imageView->destroy();
    this->imageView = nullptr;

    this->image->destroy();
    this->image = nullptr;
}
