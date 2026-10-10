#include "render/mesh.hpp"

#include <cstdio>
#include <iostream>
#include <vector>

namespace cube {
CubieMesh createCubieMesh() {
    CubieMesh mesh;
    struct Vertex {
        float px, py, pz;  // 位置
        float nx, ny, nz;  // 法线
    };
    struct FaceDef {
        float nx, ny, nz;
        float ux, uy, uz;
        float vx, vy, vz;
        bool flip;  // true = 用反序 (b+0,b+2,b+1)
    };

    constexpr FaceDef kFaces[6] = {
        // 法线      u      v        flip
        {1, 0, 0, 0, 1, 0, 0, 0, 1, true},    // +X  反序
        {-1, 0, 0, 0, 1, 0, 0, 0, 1, false},  // -X  正序
        {0, 1, 0, 0, 0, 1, 1, 0, 0, true},    // +Y  反序
        {0, -1, 0, 0, 0, 1, 1, 0, 0, false},  // -Y  正序
        {0, 0, 1, 1, 0, 0, 0, 1, 0, true},    // +Z  反序
        {0, 0, -1, 1, 0, 0, 0, 1, 0, false},  // -Z  正序
    };
    const float h = 0.5f;
    std::vector<Vertex> verts;
    std::vector<GLuint> indices;
    for (int f = 0; f < 6; f++) {
        const FaceDef& face = kFaces[f];
        const float corners[4][2] = {{-1, -1}, {1, -1}, {1, 1}, {-1, 1}};  // 面上四个角
        for (int c = 0; c < 4; ++c) {
            const float su = corners[c][0] * h;
            const float sv = corners[c][1] * h;
            Vertex v;  // 遍历定义所有顶点
            v.px = face.nx * h + face.ux * su + face.vx * sv;
            v.py = face.ny * h + face.uy * su + face.vy * sv;
            v.pz = face.nz * h + face.uz * su + face.vz * sv;
            v.nx = face.nx;
            v.ny = face.ny;
            v.nz = face.nz;
            verts.push_back(v);
        }
        const GLuint b = static_cast<GLuint>(f * 4);
        if (face.flip) {
            indices.insert(indices.end(), {b + 0, b + 2, b + 1, b + 0, b + 3, b + 2});
        } else {
            indices.insert(indices.end(), {b + 0, b + 1, b + 2, b + 0, b + 2, b + 3});
        }
    }
    for (int i = 0; i < 6; ++i) {  // 计算索引
        mesh.faceIndexOffset[i] = i * 6;
    }
    mesh.indexCount = static_cast<GLsizei>(indices.size());
    glGenBuffers(1, &mesh.vbo);
    glBindBuffer(GL_ARRAY_BUFFER, mesh.vbo);
    glBufferData(GL_ARRAY_BUFFER, verts.size() * sizeof(Vertex), verts.data(), GL_STATIC_DRAW);
    glGenVertexArrays(1, &mesh.vao);
    glBindVertexArray(mesh.vao);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)0);
    glEnableVertexAttribArray(1);
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)(3 * sizeof(float)));
    glGenBuffers(1, &mesh.ebo);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, mesh.ebo);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, indices.size() * sizeof(GLuint), indices.data(),
                 GL_STATIC_DRAW);
    glBindVertexArray(0);
    std::printf("[网格] %zu 顶点，%zu 索引，%zu 字节\n", verts.size(), indices.size(),
                verts.size() * sizeof(Vertex));
    return mesh;
}

void destroyMesh(CubieMesh& mesh) {
    if (mesh.ebo != 0)
        glDeleteBuffers(1, &mesh.ebo);
    if (mesh.vbo != 0)
        glDeleteBuffers(1, &mesh.vbo);
    if (mesh.vao != 0)
        glDeleteVertexArrays(1, &mesh.vao);
    mesh = CubieMesh{};
}

}  // namespace cube