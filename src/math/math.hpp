#pragma once
#include <glm/ext/matrix_int3x3.hpp>
#include <glm/vec3.hpp>

namespace cube {
int layerOf(int coord);
int axisIndex(char axis);
glm::imat3 rotX90();
glm::imat3 rotX(int turns);
glm::imat3 rotY90();
glm::imat3 rotY(int turns);
glm::imat3 rotZ90();
glm::imat3 rotZ(int turns);
}  // namespace cube