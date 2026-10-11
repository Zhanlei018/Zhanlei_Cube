#pragma once

#include <glm/mat4x4.hpp>
#include <glm/vec3.hpp>

#include "cube/state.hpp"

namespace cube {
struct OrbitCamera;
struct Ray {
    glm::vec3 origin{0.0f};                  // 起点（就是相机位置）
    glm::vec3 direction{0.0f, 0.0f, -1.0f};  // 方向（应该是单位向量）
};
struct PickResult {
    bool hit = false;
    float distance = 0.0f;  // 沿射线方向的距离 t（交点 = origin + direction * t）
    glm::vec3 point{0.0f};  // 交点（世界坐标）
    glm::ivec3 normal{0};
    glm::ivec3 cell{0};
};

Ray makeRayFromScreen(int mouseX, int mouseY, int width, int height, const glm::mat4& view,
                      const glm::mat4& proj);
PickResult pickCube(const Ray& ray);
PickResult pickAtScreen(int mouseX, int mouseY, int width, int height, const glm::mat4& view,
                        const glm::mat4& proj);

}  // namespace cube
