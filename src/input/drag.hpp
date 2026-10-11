#pragma once

#include <glm/mat4x4.hpp>
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

struct DragState {
    DragAnchor anchor{};     // 按下时锁定的 P0 / N / cell

    bool  valid   = false;   // 有没有锚点
    bool  active  = false;   // 拖动是否已超过阈值、预览生效
    int   axis    = 0;       // 旋转轴 0/1/2
    int   layer   = 0;       // 旋转层 0/1/2
    int   dir     = 1;       // 方向
    float angle   = 0.0f;    // 当前预览角度（弧度，可正可负）
};

void beginDrag(DragState& st, const DragAnchor& anchor);
void updateDrag(DragState& st, const Ray& ray);
void endDrag(DragState& st);
glm::mat4 dragRotationMatrix(const DragState& st);

}  // namespace cube

