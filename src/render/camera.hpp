#pragma once

#include <glm/mat4x4.hpp>
#include <glm/vec3.hpp>

namespace cube {

struct OrbitCamera {
    float yaw = 0.0f;         // 偏航角，绕Y轴方位角（弧度）
    float pitch = 0.35f;      // 俯仰角（弧度）
    float radius = 7.0f;      // 半径，到原点的距离

    int viewportWidth = 1280;
    int viewportHeight = 720;

    void orbit(float dxPixels, float dyPixels);
    void zoom(float scrollTicks);
    void setViewport(int w, int h);

    glm::vec3 eye() const;
    glm::mat4 viewMatrix() const;
    glm::mat4 projectionMatrix() const;
    glm::mat4 viewProjectionMatrix() const;
};

}  // namespace cube
