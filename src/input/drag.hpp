#pragma once

#include <glm/vec3.hpp>

#include "cube/state.hpp"
#include "input/picking.hpp"

namespace cube {
struct DragAnchor {
    bool valid = false;
    glm::vec3 point{0.0f};       // 起始命中点 P0
    glm::vec3 normal{0.0f};      // 命中面外法线 N（拖动全程不变）
    glm::ivec3 cell{0};          // P0 所在的格子
};

DragAnchor makeAnchor(const PickResult& pick);
bool intersectPlane(const Ray& ray, const glm::vec3& planePoint,
                    const glm::vec3& planeNormal, glm::vec3& outPoint);
bool makeMoveFromDrag(const DragAnchor& anchor, const Ray& releaseRay, Move& outMove);

}  // namespace cube
