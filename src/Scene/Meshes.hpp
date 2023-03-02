#ifndef SCENE_MESHES_HPP
#define SCENE_MESHES_HPP

#include <array>

#include "src/Resources/Vertex.hpp"

static constexpr std::array<Vertex, 36> CUBE_MESH = {
        // up
        Vertex{.pos = {-1, 1, 1}, .normal = {0, 1, 0}, .color = {1, 1, 1}, .uv = {0, 0}},
        Vertex{.pos = {1, 1, 1}, .normal = {0, 1, 0}, .color = {1, 1, 1}, .uv = {1, 0}},
        Vertex{.pos = {1, 1, -1}, .normal = {0, 1, 0}, .color = {1, 1, 1}, .uv = {1, 1}},
        Vertex{.pos = {1, 1, -1}, .normal = {0, 1, 0}, .color = {1, 1, 1}, .uv = {1, 1}},
        Vertex{.pos = {-1, 1, -1}, .normal = {0, 1, 0}, .color = {1, 1, 1}, .uv = {0, 1}},
        Vertex{.pos = {-1, 1, 1}, .normal = {0, 1, 0}, .color = {1, 1, 1}, .uv = {0, 0}},

        // down
        Vertex{.pos = {1, -1, -1}, .normal = {0, -1, 0}, .color = {1, 1, 1}, .uv = {0, 0}},
        Vertex{.pos = {1, -1, 1}, .normal = {0, -1, 0}, .color = {1, 1, 1}, .uv = {1, 0}},
        Vertex{.pos = {-1, -1, 1}, .normal = {0, -1, 0}, .color = {1, 1, 1}, .uv = {1, 1}},
        Vertex{.pos = {-1, -1, 1}, .normal = {0, -1, 0}, .color = {1, 1, 1}, .uv = {1, 1}},
        Vertex{.pos = {-1, -1, -1}, .normal = {0, -1, 0}, .color = {1, 1, 1}, .uv = {0, 1}},
        Vertex{.pos = {1, -1, -1}, .normal = {0, -1, 0}, .color = {1, 1, 1}, .uv = {0, 0}},

        // left
        Vertex{.pos = {-1, 1, 1}, .normal = {-1, 0, 0}, .color = {1, 1, 1}, .uv = {0, 0}},
        Vertex{.pos = {-1, 1, -1}, .normal = {-1, 0, 0}, .color = {1, 1, 1}, .uv = {1, 0}},
        Vertex{.pos = {-1, -1, -1}, .normal = {-1, 0, 0}, .color = {1, 1, 1}, .uv = {1, 1}},
        Vertex{.pos = {-1, -1, -1}, .normal = {-1, 0, 0}, .color = {1, 1, 1}, .uv = {1, 1}},
        Vertex{.pos = {-1, -1, 1}, .normal = {-1, 0, 0}, .color = {1, 1, 1}, .uv = {0, 1}},
        Vertex{.pos = {-1, 1, 1}, .normal = {-1, 0, 0}, .color = {1, 1, 1}, .uv = {0, 0}},

        // right
        Vertex{.pos = {1, 1, -1}, .normal = {1, 0, 0}, .color = {1, 1, 1}, .uv = {0, 0}},
        Vertex{.pos = {1, 1, 1}, .normal = {1, 0, 0}, .color = {1, 1, 1}, .uv = {1, 0}},
        Vertex{.pos = {1, -1, 1}, .normal = {1, 0, 0}, .color = {1, 1, 1}, .uv = {1, 1}},
        Vertex{.pos = {1, -1, 1}, .normal = {1, 0, 0}, .color = {1, 1, 1}, .uv = {1, 1}},
        Vertex{.pos = {1, -1, -1}, .normal = {1, 0, 0}, .color = {1, 1, 1}, .uv = {0, 1}},
        Vertex{.pos = {1, 1, -1}, .normal = {1, 0, 0}, .color = {1, 1, 1}, .uv = {0, 0}},

        // front
        Vertex{.pos = {1, 1, 1}, .normal = {0, 0, 1}, .color = {1, 1, 1}, .uv = {0, 0}},
        Vertex{.pos = {-1, 1, 1}, .normal = {0, 0, 1}, .color = {1, 1, 1}, .uv = {1, 0}},
        Vertex{.pos = {-1, -1, 1}, .normal = {0, 0, 1}, .color = {1, 1, 1}, .uv = {1, 1}},
        Vertex{.pos = {-1, -1, 1}, .normal = {0, 0, 1}, .color = {1, 1, 1}, .uv = {1, 1}},
        Vertex{.pos = {1, -1, 1}, .normal = {0, 0, 1}, .color = {1, 1, 1}, .uv = {0, 1}},
        Vertex{.pos = {1, 1, 1}, .normal = {0, 0, 1}, .color = {1, 1, 1}, .uv = {0, 0}},

        // back
        Vertex{.pos = {-1, 1, -1}, .normal = {0, 0, -1}, .color = {1, 1, 1}, .uv = {0, 0}},
        Vertex{.pos = {1, 1, -1}, .normal = {0, 0, -1}, .color = {1, 1, 1}, .uv = {1, 0}},
        Vertex{.pos = {1, -1, -1}, .normal = {0, 0, -1}, .color = {1, 1, 1}, .uv = {1, 1}},
        Vertex{.pos = {1, -1, -1}, .normal = {0, 0, -1}, .color = {1, 1, 1}, .uv = {1, 1}},
        Vertex{.pos = {-1, -1, -1}, .normal = {0, 0, -1}, .color = {1, 1, 1}, .uv = {0, 1}},
        Vertex{.pos = {-1, 1, -1}, .normal = {0, 0, -1}, .color = {1, 1, 1}, .uv = {0, 0}}
};

static constexpr std::array<Vertex, 36> SKYBOX_MESH = {
        // up
        Vertex{.pos = {-1, 1, -1}},
        Vertex{.pos = {1, 1, -1}},
        Vertex{.pos = {1, 1, 1}},
        Vertex{.pos = {1, 1, 1}},
        Vertex{.pos = {-1, 1, 1}},
        Vertex{.pos = {-1, 1, -1}},

        // front
        Vertex{.pos = {-1, 1, 1}},
        Vertex{.pos = {1, 1, 1}},
        Vertex{.pos = {1, -1, 1}},
        Vertex{.pos = {1, -1, 1}},
        Vertex{.pos = {-1, -1, 1}},
        Vertex{.pos = {-1, 1, 1}},

        // left
        Vertex{.pos = {-1, 1, -1}},
        Vertex{.pos = {-1, 1, 1}},
        Vertex{.pos = {-1, -1, 1}},
        Vertex{.pos = {-1, -1, 1}},
        Vertex{.pos = {-1, -1, -1}},
        Vertex{.pos = {-1, 1, -1}},

        // right
        Vertex{.pos = {1, 1, 1}},
        Vertex{.pos = {1, 1, -1}},
        Vertex{.pos = {1, -1, -1}},
        Vertex{.pos = {1, -1, -1}},
        Vertex{.pos = {1, -1, 1}},
        Vertex{.pos = {1, 1, 1}},

        // back
        Vertex{.pos = {1, 1, -1}},
        Vertex{.pos = {-1, 1, -1}},
        Vertex{.pos = {-1, -1, -1}},
        Vertex{.pos = {-1, -1, -1}},
        Vertex{.pos = {1, -1, -1}},
        Vertex{.pos = {1, 1, -1}},

        // down
        Vertex{.pos = {-1, -1, 1}},
        Vertex{.pos = {1, -1, 1}},
        Vertex{.pos = {1, -1, -1}},
        Vertex{.pos = {1, -1, -1}},
        Vertex{.pos = {-1, -1, -1}},
        Vertex{.pos = {-1, -1, 1}},
};

#endif // SCENE_MESHES_HPP
