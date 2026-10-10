#pragma once

#include <glm/ext/matrix_int3x3.hpp>
#include <glm/vec3.hpp>

namespace cube {
struct Cubie {
    glm::ivec3 pos;
    glm::imat3 orient;
};
Cubie makeSolvedCubie(glm::ivec3 pos);
}  // namespace cube
