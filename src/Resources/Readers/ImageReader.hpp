#ifndef SRC_RESOURCES_READERS_IMAGEREADER_HPP
#define SRC_RESOURCES_READERS_IMAGEREADER_HPP

#include <memory>
#include <optional>

class Log;
class ResourceData;

struct ImageData {
    ~ImageData();

    uint32_t width;
    uint32_t height;
    uint32_t channels;
    void *image;
};

class ImageReader {
private:
    std::shared_ptr<Log> _log;

    std::unique_ptr<ImageData> read(const std::weak_ptr<ResourceData> &resourceData);

public:
    ImageReader(const std::shared_ptr<Log> &log);

    [[nodiscard]] std::optional<std::unique_ptr<ImageData>> tryRead(const std::weak_ptr<ResourceData> &resourceData);
};

#endif // SRC_RESOURCES_READERS_IMAGEREADER_HPP
