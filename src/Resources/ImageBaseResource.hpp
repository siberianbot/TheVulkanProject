#ifndef RESOURCES_IMAGEBASERESOURCE_HPP
#define RESOURCES_IMAGEBASERESOURCE_HPP

#include <memory>

#include "Resource.hpp"

class RenderingObjectsAllocator;
class ImageObject;

class ImageBaseResource : public Resource {
protected:
    struct ImageData {
        int width;
        int height;
        int channels;
        void *data;
    };

    std::shared_ptr<RenderingObjectsAllocator> _renderingObjectsAllocator;

    std::shared_ptr<ImageObject> _image;

    ImageBaseResource(const std::string &id, const std::vector<std::filesystem::path> &paths,
                      const std::shared_ptr<RenderingObjectsAllocator> &renderingObjectsAllocator);

    ImageData loadImageData(const std::filesystem::path &path);

    virtual std::shared_ptr<ImageObject> loadImage() = 0;

public:
    ~ImageBaseResource() override = default;

    void load() override;
    void unload() override;

    [[nodiscard]] std::shared_ptr<ImageObject> image() const { return this->_image; }
};

#endif // RESOURCES_IMAGEBASERESOURCE_HPP
