#include "camera.hpp"
#include <algorithm>
#include <cmath>
#include <glm/gtc/matrix_transform.hpp>

namespace cube {
glm::vec3 OrbitCamera::eye() const {  // 把球坐标转换为笛卡尔坐标系的坐标
    float x = radius * cos(pitch) * sin(yaw);
    float y = radius * sin(pitch);
    float z = radius * cos(pitch) * cos(yaw);
    return glm::vec3(x, y, z);
}

glm::mat4 OrbitCamera::viewMatrix() const {
    return glm::lookAt(eye(), glm::vec3(0.0f), glm::vec3(0.0f, 1.0f, 0.0f));
}

glm::mat4 OrbitCamera::projectionMatrix() const {
    const float aspect = (float)viewportWidth / (float)viewportHeight;
    return glm::perspective(glm::radians(45.0f), aspect, 0.1f, 100.0f);
}

glm::mat4 OrbitCamera::viewProjectionMatrix() const {
    return projectionMatrix() * viewMatrix();
}
void OrbitCamera::orbit(float dxPixels, float dyPixels) {
    const float sensibility = 0.005f;
    yaw -= dxPixels * sensibility;
    pitch -= dyPixels * sensibility;
    const float maxPitch = glm::radians(89.0f);
    pitch = std::clamp(pitch, -maxPitch, maxPitch);
}

void OrbitCamera::zoom(float scrollTicks){
    const float zoomSpeed = 0.5f;
    radius -= scrollTicks * zoomSpeed;
    if (radius < 2.0f) radius = 2.0f;
    if (radius > 20.0f) radius = 20.0f;
}

void OrbitCamera::setViewport(int w, int h){
    viewportWidth  = (w > 0) ? w : 1;
    viewportHeight = (h > 0) ? h : 1;
}
}  // namespace cube