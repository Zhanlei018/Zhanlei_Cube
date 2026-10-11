#include "picking.hpp"
#include <algorithm>

namespace cube {
Ray makeRayFromScreen(int mouseX, int mouseY, int width, int height, const glm::mat4& view,
                      const glm::mat4& proj) {
    float x = (2.0f * mouseX) / static_cast<float>(width) - 1.0f;
    float y = 1.0f - (2.0f * mouseY) / static_cast<float>(height);
    glm::vec4 rayClip(x, y, -1.0f, 1.0f);
    glm::vec4 rayEye = glm::inverse(proj) * rayClip;
    rayEye = glm::vec4(rayEye.x, rayEye.y, -1.0f, 0.0f);
    glm::vec4 rayWorld = glm::inverse(view) * rayEye;
    glm::vec3 rayDir = glm::normalize(glm::vec3(rayWorld));
    glm::vec3 rayOrigin = glm::vec3(glm::inverse(view)[3]);
    return {rayOrigin, rayDir};
}

PickResult pickCube(const Ray& ray){
    PickResult result;
    result.hit = false;
    const glm::vec3 minBounds(-1.5f);
    const glm::vec3 maxBounds(1.5f);
    const float infinity = std::numeric_limits<float>::infinity();
    glm::vec3 dirInv(
        ray.direction.x != 0.0f ? 1.0f / ray.direction.x : infinity,
        ray.direction.y != 0.0f ? 1.0f / ray.direction.y : infinity,
        ray.direction.z != 0.0f ? 1.0f / ray.direction.z : infinity
    );
    glm::vec3 tmin_v = (minBounds - ray.origin) * dirInv;
    glm::vec3 tmax_v = (maxBounds - ray.origin) * dirInv;
    glm::vec3 t1 = glm::min(tmin_v, tmax_v);
    glm::vec3 t2 = glm::max(tmin_v, tmax_v);
    float tmin = std::max({t1.x, t1.y, t1.z});
    float tmax = std::min({t2.x, t2.y, t2.z});
    if (tmax < 0.0f || tmin > tmax) {
        return result;
    }
    float t = tmin;
    if (t < 0.0f) t = tmax;
    result.hit = true;
    result.distance = t;
    result.point = ray.origin + ray.direction * t;
    if (tmin == t1.x)
    {
        result.normal = (ray.direction.x > 0.0f) ? glm::ivec3(-1, 0, 0) : glm::ivec3(1, 0, 0);
    }else if (tmin == t1.y) {
        result.normal = (ray.direction.y > 0.0f) ? glm::ivec3(0, -1, 0) : glm::ivec3(0, 1, 0);
    } else {
        result.normal = (ray.direction.z > 0.0f) ? glm::ivec3(0, 0, -1) : glm::ivec3(0, 0, 1);
    }
    result.cell = glm::clamp(glm::ivec3(glm::floor(result.point + glm::vec3(0.5f))),
                         glm::ivec3(-1), glm::ivec3(1));
    return result;
}

PickResult pickAtScreen(int mouseX, int mouseY, int width, int height, const glm::mat4& view,
                        const glm::mat4& proj) {
    Ray ray = makeRayFromScreen(mouseX, mouseY, width, height, view, proj);
    return pickCube(ray);
}
}  // namespace cube