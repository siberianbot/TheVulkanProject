#ifndef SRC_SCENE_SKYBOXMESH_HPP
#define SRC_SCENE_SKYBOXMESH_HPP

#include <array>

#include "src/Resources/Mesh.hpp"

static constexpr std::array<Vertex, 36> DEFAULT_SKYBOX_MESH = {
        // up
        Vertex{.pos = {-1, 1, -1}, .color = {1, 1, 1}, .texCoord = {0.25, 0}},
        Vertex{.pos = {1, 1, -1}, .color = {1, 1, 1}, .texCoord = {0.5, 0}},
        Vertex{.pos = {1, 1, 1}, .color = {1, 1, 1}, .texCoord = {0.5, 1.0 / 3}},
        Vertex{.pos = {1, 1, 1}, .color = {1, 1, 1}, .texCoord = {0.5, 1.0 / 3}},
        Vertex{.pos = {-1, 1, 1}, .color = {1, 1, 1}, .texCoord = {0.25, 1.0 / 3}},
        Vertex{.pos = {-1, 1, -1}, .color = {1, 1, 1}, .texCoord = {0.25, 0}},

        // front
        Vertex{.pos = {-1, 1, 1}, .color = {1, 1, 1}, .texCoord = {0.25, 1.0 / 3}},
        Vertex{.pos = {1, 1, 1}, .color = {1, 1, 1}, .texCoord = {0.5, 1.0 / 3}},
        Vertex{.pos = {1, -1, 1}, .color = {1, 1, 1}, .texCoord = {0.5, 2.0 / 3}},
        Vertex{.pos = {1, -1, 1}, .color = {1, 1, 1}, .texCoord = {0.5, 2.0 / 3}},
        Vertex{.pos = {-1, -1, 1}, .color = {1, 1, 1}, .texCoord = {0.25, 2.0 / 3}},
        Vertex{.pos = {-1, 1, 1}, .color = {1, 1, 1}, .texCoord = {0.25, 1.0 / 3}},

        // left
        Vertex{.pos = {-1, 1, -1}, .color = {1, 1, 1}, .texCoord = {0, 1.0 / 3}},
        Vertex{.pos = {-1, 1, 1}, .color = {1, 1, 1}, .texCoord = {0.25, 1.0 / 3}},
        Vertex{.pos = {-1, -1, 1}, .color = {1, 1, 1}, .texCoord = {0.25, 2.0 / 3}},
        Vertex{.pos = {-1, -1, 1}, .color = {1, 1, 1}, .texCoord = {0.25, 2.0 / 3}},
        Vertex{.pos = {-1, -1, -1}, .color = {1, 1, 1}, .texCoord = {0, 2.0 / 3}},
        Vertex{.pos = {-1, 1, -1}, .color = {1, 1, 1}, .texCoord = {0, 1.0 / 3}},

        // right
        Vertex{.pos = {1, 1, 1}, .color = {1, 1, 1}, .texCoord = {0.5, 1.0 / 3}},
        Vertex{.pos = {1, 1, -1}, .color = {1, 1, 1}, .texCoord = {0.75, 1.0 / 3}},
        Vertex{.pos = {1, -1, -1}, .color = {1, 1, 1}, .texCoord = {0.75, 2.0 / 3}},
        Vertex{.pos = {1, -1, -1}, .color = {1, 1, 1}, .texCoord = {0.75, 2.0 / 3}},
        Vertex{.pos = {1, -1, 1}, .color = {1, 1, 1}, .texCoord = {0.5, 2.0 / 3}},
        Vertex{.pos = {1, 1, 1}, .color = {1, 1, 1}, .texCoord = {0.5, 1.0 / 3}},

        // back
        Vertex{.pos = {1, 1, -1}, .color = {1, 1, 1}, .texCoord = {0.75, 1.0 / 3}},
        Vertex{.pos = {-1, 1, -1}, .color = {1, 1, 1}, .texCoord = {1, 1.0 / 3}},
        Vertex{.pos = {-1, -1, -1}, .color = {1, 1, 1}, .texCoord = {1, 2.0 / 3}},
        Vertex{.pos = {-1, -1, -1}, .color = {1, 1, 1}, .texCoord = {1, 2.0 / 3}},
        Vertex{.pos = {1, -1, -1}, .color = {1, 1, 1}, .texCoord = {0.75, 2.0 / 3}},
        Vertex{.pos = {1, 1, -1}, .color = {1, 1, 1}, .texCoord = {0.75, 1.0 / 3}},

        // down
        Vertex{.pos = {-1, -1, 1}, .color = {1, 1, 1}, .texCoord = {0.25, 2.0 / 3}},
        Vertex{.pos = {1, -1, 1}, .color = {1, 1, 1}, .texCoord = {0.5, 2.0 / 3}},
        Vertex{.pos = {1, -1, -1}, .color = {1, 1, 1}, .texCoord = {0.5, 1}},
        Vertex{.pos = {1, -1, -1}, .color = {1, 1, 1}, .texCoord = {0.5, 1}},
        Vertex{.pos = {-1, -1, -1}, .color = {1, 1, 1}, .texCoord = {0.25, 1}},
        Vertex{.pos = {-1, -1, 1}, .color = {1, 1, 1}, .texCoord = {0.25, 2.0 / 3}},
};

#endif // SRC_SCENE_SKYBOXMESH_HPP
