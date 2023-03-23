#ifndef RESOURCES_IMAGERESOURCE_HPP
#define RESOURCES_IMAGERESOURCE_HPP

#include "ImageBaseResource.hpp"

class ImageResource : public ImageBaseResource {
private:
    std::shared_ptr<ImageObject> loadImage() override;

public:
    ImageResource(const std::string &id, const std::filesystem::path &path,
                  const std::shared_ptr<RenderingObjectsAllocator> &renderingObjectsAllocator);
    ~ImageResource() override = default;

    [[nodiscard]] ResourceType type() const override { return IMAGE_RESOURCE; }
};

#endif // RESOURCES_IMAGERESOURCE_HPP
