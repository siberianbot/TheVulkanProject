#include "Texture.hpp"

#include <stdexcept>

#define STB_IMAGE_IMPLEMENTATION
#include <stb/stb_image.h>

Texture::~Texture() {
    if (this->_pixels != nullptr) {
        stbi_image_free(this->_pixels);
    }
}

Texture *Texture::fromFile(const std::filesystem::path &path) {
    Texture *texture = new Texture;
    int channels;

    texture->_pixels = stbi_load(path.c_str(), &texture->_width, &texture->_height, &channels, STBI_rgb_alpha);

    if (texture->_pixels == nullptr) {
        throw std::runtime_error("Failed to read image");
    }

    return texture;
}
