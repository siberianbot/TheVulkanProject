#include "Skybox.hpp"

Skybox::Skybox(const std::shared_ptr<MeshResource> &mesh, const std::shared_ptr<CubeImageResource> &texture)
        : _mesh(mesh),
          _texture(texture) {
    //
}
