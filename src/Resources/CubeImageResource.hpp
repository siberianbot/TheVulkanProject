#ifndef RESOURCES_CUBEIMAGERESOURCE_HPP
#define RESOURCES_CUBEIMAGERESOURCE_HPP

#include <array>

#include "ImageBaseResource.hpp"

class CubeImageResource : public ImageBaseResource {
private:
    std::shared_ptr<ImageObject> loadImage() override;

public:
    CubeImageResource(const std::string &id, const std::array<std::filesystem::path, 6> &paths,
                      const std::shared_ptr<RendererAllocator> &rendererAllocator);
    ~CubeImageResource() override = default;

    [[nodiscard]] ResourceType type() const override { return CUBE_IMAGE_RESOURCE; }
};

#endif // RESOURCES_CUBEIMAGERESOURCE_HPP
