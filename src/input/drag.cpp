#include "math/math.hpp"
#include "input/drag.hpp"

namespace cube {

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
    float denominator = glm::dot(ray.direction, planeNormal);
    if (std::abs(denominator) < 1e-6f) {
        return false;
    }
    float t = -glm::dot(ray.origin - planePoint, planeNormal) / denominator;
    if (t < 0.0f) {
        return false;
    }
    outPoint = ray.origin + t * ray.direction;
    return true;
}

bool makeMoveFromDrag(const DragAnchor& anchor, const Ray& releaseRay, Move& outMove) {
    if (!anchor.valid)
        return false;
    glm::vec3 P1;
    if (!intersectPlane(releaseRay, anchor.point, anchor.normal, P1)) {
        return false;
    }
    glm::vec3 D = P1 - anchor.point;
    if (glm::length(D) < 0.15f) {
        return false;
    }
    glm::vec3 Dt = D - anchor.normal * glm::dot(D, anchor.normal);
    if (glm::length(Dt) < 0.01f)
        return false;
    glm::vec3 dragAxis(0.0f);
    float absX = std::abs(Dt.x);
    float absY = std::abs(Dt.y);
    float absZ = std::abs(Dt.z);
    if (absX >= absY && absX >= absZ) {
        dragAxis.x = (Dt.x > 0.0f) ? 1.0f : -1.0f;
    } else if (absY >= absX && absY >= absZ) {
        dragAxis.y = (Dt.y > 0.0f) ? 1.0f : -1.0f;
    } else {
        dragAxis.z = (Dt.z > 0.0f) ? 1.0f : -1.0f;
    }
    glm::vec3 rotAxisVec = glm::cross(anchor.normal, dragAxis);
    glm::ivec3 rotAxis = glm::ivec3(glm::round(rotAxisVec));
    int axisIndex = -1;
    if (rotAxis.x != 0) axisIndex = 0;      // X 轴
    else if (rotAxis.y != 0) axisIndex = 1; // Y 轴
    else if (rotAxis.z != 0) axisIndex = 2; // Z 轴
    else return false;
    int layerVal = 0;
    if (axisIndex == 0) layerVal = layerOf(anchor.cell.x);
    else if (axisIndex == 1) layerVal = layerOf(anchor.cell.y);
    else if (axisIndex == 2) layerVal = layerOf(anchor.cell.z);
    int dir = 1;
    if (axisIndex == 0) dir = (rotAxis.x > 0) ? 1 : -1;
    else if (axisIndex == 1) dir = (rotAxis.y > 0) ? 1 : -1;
    else if (axisIndex == 2) dir = (rotAxis.z > 0) ? 1 : -1;
    outMove.axis = axisIndex;
    outMove.layer = layerVal;
    outMove.dir = dir;
    outMove.turns = 1;
    return true; 
}
}  // namespace cube