#ifndef RESOURCES_TEXTURE_HPP
#define RESOURCES_TEXTURE_HPP

#include <string>

class Texture {
private:
    int _width;
    int _height;
    void *_pixels = nullptr;

    Texture() = default;

public:
    ~Texture();

    [[nodiscard]] uint32_t width() const { return this->_width; }
    [[nodiscard]] uint32_t height() const { return this->_height; }
    [[nodiscard]] uint32_t size() const { return this->_width * this->_height * 4 /* channels */; }

    [[nodiscard]] void *data() const { return this->_pixels; }

    [[nodiscard]] static Texture fromFile(const std::string &path);
};

#endif // RESOURCES_TEXTURE_HPP
