#include "ImageReader.hpp"

#define STB_IMAGE_IMPLEMENTATION
#include <stb/stb_image.h>

#include "src/Engine/EngineError.hpp"
#include "src/Engine/Log.hpp"
#include "src/Resources/ResourceData.hpp"
#include "src/Utils/DataStream.hpp"

static constexpr const char *IMAGE_READER_TAG = "ImageReader";

ImageData::~ImageData() {
    stbi_image_free(this->image);
}

std::unique_ptr<ImageData> ImageReader::read(const std::weak_ptr<ResourceData> &resourceData) {
    std::shared_ptr<ResourceData> lockedResourceData = resourceData.lock();
    std::unique_ptr<ImageData> imageData = std::make_unique<ImageData>();

    int width, height, channels;
    imageData->image = stbi_load_from_memory(reinterpret_cast<const unsigned char *>(lockedResourceData->data().data()),
                                             lockedResourceData->data().size(), &width, &height, &channels,
                                             STBI_rgb_alpha);

    if (imageData->image == nullptr) {
        throw EngineError("Failed to read image data");
    }

    imageData->width = width;
    imageData->height = height;
    imageData->channels = channels;

    return imageData;
}

ImageReader::ImageReader(const std::shared_ptr<Log> &log)
        : _log(log) {
    //
}

std::optional<std::unique_ptr<ImageData>> ImageReader::tryRead(const std::weak_ptr<ResourceData> &resourceData) {
    try {
        return this->read(resourceData);
    } catch (const std::exception &error) {
        this->_log->error(IMAGE_READER_TAG, error);
        return std::nullopt;
    }
}
