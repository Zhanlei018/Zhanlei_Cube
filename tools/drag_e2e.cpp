#include "input/drag.hpp"
#include "anim/animator.hpp"
#include <glm/gtc/matrix_transform.hpp>
#include <cstdio>
#include <cmath>

struct Cam {
    float pitch = 0.35f, radius = 7.0f;
    int w = 1280, h = 720;
    glm::vec3 eye() const { float cp=std::cos(pitch); return {0.0f, radius*std::sin(pitch), radius*cp}; }
    glm::mat4 view() const { return glm::lookAt(eye(), glm::vec3(0.0f), glm::vec3(0,1,0)); }
    glm::mat4 proj() const {
        return glm::perspective(glm::radians(45.0f), (float)w/(float)h, 0.1f, 100.0f); }
};

int main() {
    Cam cam;
    int px=-1, py=-1; cube::PickResult hit;
    for (int y = 260; y <= 460 && px < 0; y += 4)
        for (int x = 540; x <= 740; x += 4) {
            auto p = cube::pickAtScreen(x,y,cam.w,cam.h,cam.view(),cam.proj());
            if (p.hit) { px=x; py=y; hit=p; break; }
        }
    cube::DragAnchor anchor = cube::makeAnchor(hit);
    cube::DragState st;

    std::printf("=== 交接检查：同一进度下，预览角度 vs 动画角度 ===\n\n");
    std::printf(" 像素偏移  预览(度)  动画(度)   差值(度)\n");

    for (int dx = 40; dx <= 200; dx += 20) {
        cube::beginDrag(st, anchor);
        cube::Ray r = cube::makeRayFromScreen(px+dx, py, cam.w, cam.h, cam.view(), cam.proj());
        cube::updateDrag(st, r);
        if (!st.active) continue;

        const float previewDeg = std::abs(st.angle) * 180.0f / 3.14159265f;
        const float progress = std::abs(st.angle) / glm::half_pi<float>();

        cube::Animator anim(0.20f);
        cube::Move m;
        m.axis = st.axis; m.layer = st.layer; m.dir = st.dir; m.turns = 1;
        anim.beginMove(m, progress);

        // 从动画矩阵里反推角度：把 (1,0,0) 绕 axis 转过的角度
        const glm::mat4 M = anim.rotationMatrix();
        const int a = st.axis;
        const glm::vec3 basis = (a == 0) ? glm::vec3(0,1,0) : glm::vec3(0,0,1);
        const glm::vec3 rotated = glm::mat3(M) * basis;
        const float animDeg = std::atan2(glm::length(glm::cross(basis, rotated)),
                                         glm::dot(basis, rotated)) * 180.0f / 3.14159265f;

        const float diff = std::abs(previewDeg - animDeg);
        std::printf("   %4d     %7.2f   %7.2f   %7.3f   %s\n",
                    dx, previewDeg, animDeg, diff, diff < 0.5f ? "OK" : "<<< 跳变!");
        cube::endDrag(st);
    }
    return 0;
}
