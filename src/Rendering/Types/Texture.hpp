#ifndef RENDERING_TYPES_TEXTURE_HPP
#define RENDERING_TYPES_TEXTURE_HPP

#include <memory>

#include "src/Rendering/Types/ImageView.hpp"

struct Texture {
    std::weak_ptr<ImageView> image;
};

#endif // RENDERING_TYPES_TEXTURE_HPP
