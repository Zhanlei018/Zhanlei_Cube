#include "input/drag.hpp"

#include <algorithm>
#include <cmath>

#include <glm/gtc/quaternion.hpp>

#include "math/math.hpp"

namespace cube {

namespace {
constexpr float kHoldProgress = 0.6f;
constexpr float kCellSize = 1.0f;
const float kMaxPreviewAngle = glm::half_pi<float>();

bool moveFromAnchorPoint(const DragAnchor& anchor, const glm::vec3& point, Move& outMove) {
    const glm::vec3 D = point - anchor.point;
    if (glm::length(D) < 0.15f) {
        return false;
    }
    const glm::vec3 Dt = D - anchor.normal * glm::dot(D, anchor.normal);
    if (glm::length(Dt) < 0.01f) {
        return false;
    }

    glm::vec3 dragAxis(0.0f);
    const float ax = std::abs(Dt.x);
    const float ay = std::abs(Dt.y);
    const float az = std::abs(Dt.z);
    if (ax >= ay && ax >= az) {
        dragAxis.x = (Dt.x > 0.0f) ? 1.0f : -1.0f;
    } else if (ay >= ax && ay >= az) {
        dragAxis.y = (Dt.y > 0.0f) ? 1.0f : -1.0f;
    } else {
        dragAxis.z = (Dt.z > 0.0f) ? 1.0f : -1.0f;
    }

    const glm::vec3 rotVec = glm::cross(anchor.normal, dragAxis);
    const glm::ivec3 rot = glm::ivec3(glm::round(rotVec));

    int axis = -1;
    if (rot.x != 0) {
        axis = 0;
    } else if (rot.y != 0) {
        axis = 1;
    } else if (rot.z != 0) {
        axis = 2;
    } else {
        return false;
    }

    int coord = 0;
    if (axis == 0) {
        coord = anchor.cell.x;
    } else if (axis == 1) {
        coord = anchor.cell.y;
    } else {
        coord = anchor.cell.z;
    }

    int dir = 1;
    if (axis == 0) {
        dir = (rot.x > 0) ? 1 : -1;
    } else if (axis == 1) {
        dir = (rot.y > 0) ? 1 : -1;
    } else {
        dir = (rot.z > 0) ? 1 : -1;
    }

    outMove.axis = axis;
    outMove.layer = layerOf(coord);
    outMove.dir = dir;
    outMove.turns = 1;
    return true;
}

}  // namespace

DragAnchor makeAnchor(const PickResult& pick) {
    DragAnchor anchor;
    if (!pick.hit) {
        anchor.valid = false;
        return anchor;
    }
    anchor.valid = true;
    anchor.point = pick.point;
    anchor.normal = glm::vec3(pick.normal);
    anchor.cell = pick.cell;
    return anchor;
}

bool intersectPlane(const Ray& ray, const glm::vec3& planePoint, const glm::vec3& planeNormal,
                    glm::vec3& outPoint) {
    const float denominator = glm::dot(ray.direction, planeNormal);
    if (std::abs(denominator) < 1e-6f) {
        return false;
    }
    const float t = -glm::dot(ray.origin - planePoint, planeNormal) / denominator;
    if (t < 0.0f) {
        return false;
    }
    outPoint = ray.origin + t * ray.direction;
    return true;
}

bool makeMoveFromDrag(const DragAnchor& anchor, const Ray& releaseRay, Move& outMove) {
    if (!anchor.valid) {
        return false;
    }
    glm::vec3 P1;
    if (!intersectPlane(releaseRay, anchor.point, anchor.normal, P1)) {
        return false;
    }
    return moveFromAnchorPoint(anchor, P1, outMove);
}

void beginDrag(DragState& st, const DragAnchor& anchor) {
    const bool wasActive = st.active;
    const int prevAxis = st.axis;
    const int prevLayer = st.layer;
    const int prevDir = st.dir;

    st = DragState{};
    st.anchor = anchor;
    st.valid = anchor.valid;

    if (!anchor.valid) {
        return;
    }

    Move m;
    if (wasActive && moveFromAnchorPoint(anchor, anchor.point, m) && m.axis == prevAxis &&
        m.layer == prevLayer && m.dir == prevDir) {
        st.active = true;
        st.axis = m.axis;
        st.layer = m.layer;
        st.dir = m.dir;
    }
}

void updateDrag(DragState& st, const Ray& ray) {
    if (!st.valid) {
        st.active = false;
        st.angle = 0.0f;
        return;
    }

    glm::vec3 P1;
    if (!intersectPlane(ray, st.anchor.point, st.anchor.normal, P1)) {
        st.active = false;
        st.angle = 0.0f;
        return;
    }

    const glm::vec3 D = P1 - st.anchor.point;
    const glm::vec3 Dt = D - st.anchor.normal * glm::dot(D, st.anchor.normal);
    if (glm::length(Dt) < 1e-4f) {
        st.active = false;
        st.angle = 0.0f;
        return;
    }

    if (!st.active) {
        Move m;
        if (!moveFromAnchorPoint(st.anchor, P1, m)) {
            return;
        }
        st.active = true;
        st.axis = m.axis;
        st.layer = m.layer;
        st.dir = m.dir;
    }
    
    glm::vec3 dragAxis(0.0f);
    const float ax = std::abs(Dt.x);
    const float ay = std::abs(Dt.y);
    const float az = std::abs(Dt.z);
    if (ax >= ay && ax >= az) {
        dragAxis.x = (Dt.x > 0.0f) ? 1.0f : -1.0f;
    } else if (ay >= ax && ay >= az) {
        dragAxis.y = (Dt.y > 0.0f) ? 1.0f : -1.0f;
    } else {
        dragAxis.z = (Dt.z > 0.0f) ? 1.0f : -1.0f;
    }

    const float p = std::clamp(glm::dot(Dt, dragAxis) / kCellSize, -1.0f, 1.0f);

    const float sign = (p >= 0.0f) ? 1.0f : -1.0f;
    const float mag = std::abs(p);

    const float u = std::min(mag / kHoldProgress, 1.0f);

    st.angle = sign * u * kMaxPreviewAngle;
}

void endDrag(DragState& st) {
    st = DragState{};
}

glm::mat4 dragRotationMatrix(const DragState& st) {
    if (!st.valid || !st.active || std::abs(st.angle) < 1e-5f) {
        return glm::mat4(1.0f);
    }
    glm::vec3 axis(0.0f);
    axis[st.axis] = 1.0f;
    const float signedAngle = static_cast<float>(st.dir) * st.angle;
    return glm::mat4_cast(glm::angleAxis(signedAngle, axis));
}

}  // namespace cube
