#ifndef RESOURCES_IMAGERESOURCE_HPP
#define RESOURCES_IMAGERESOURCE_HPP

#include "ImageBaseResource.hpp"

class ImageResource : public ImageBaseResource {
private:
    std::shared_ptr<ImageObject> loadImage() override;

public:
    ImageResource(const std::filesystem::path &path,
                  const std::shared_ptr<RendererAllocator> &rendererAllocator);
    ~ImageResource() override = default;

    [[nodiscard]] ResourceType type() const override { return IMAGE_RESOURCE; }
};

#endif // RESOURCES_IMAGERESOURCE_HPP
