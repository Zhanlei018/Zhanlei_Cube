#pragma once

#include "render/gl_headers.hpp"

namespace cube {
struct CubieMesh {
    GLuint vao = 0;
    GLuint vbo = 0;
    GLuint ebo = 0;
    GLsizei indexCount = 0;
    GLsizei faceIndexOffset[6] = {};
};
CubieMesh createCubieMesh();
void destroyMesh(CubieMesh& mesh);
}  // namespace cube
