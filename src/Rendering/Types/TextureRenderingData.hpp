#ifndef RENDERING_TYPES_TEXTURERENDERINGDATA_HPP
#define RENDERING_TYPES_TEXTURERENDERINGDATA_HPP

#include <memory>

class ImageObject;
class ImageViewObject;

struct TextureRenderingData {
    std::shared_ptr<ImageObject> image;
    std::shared_ptr<ImageViewObject> imageView;

    [[nodiscard]] bool isValid() {
        return this->image != nullptr && this->imageView != nullptr;
    }

    void free();
};

#endif // RENDERING_TYPES_TEXTURERENDERINGDATA_HPP
