#ifndef RESOURCES_IMAGEBASERESOURCE_HPP
#define RESOURCES_IMAGEBASERESOURCE_HPP

#include "Resource.hpp"

class RendererAllocator;
class ImageObject;

class ImageBaseResource : public Resource {
protected:
    struct ImageData {
        int width;
        int height;
        int channels;
        void *data;
    };

    RendererAllocator *_rendererAllocator;

    ImageObject *_image = nullptr;

    ImageBaseResource(const std::vector<std::filesystem::path> &paths, RendererAllocator *rendererAllocator);

    ImageData loadImageData(const std::filesystem::path &path);
    virtual ImageObject *loadImage() = 0;

public:
    ~ImageBaseResource() override = default;

    void load() override;
    void unload() override;

    [[nodiscard]] ImageObject *image() const { return this->_image; }
};

#endif // RESOURCES_IMAGEBASERESOURCE_HPP
