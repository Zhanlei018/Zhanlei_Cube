// tools/drag_check.cpp —— 拖动→Move 的映射验证
//
// 【你能用它验证什么】
//   1. intersectPlane：射线和平面求交，这个纯几何，能完全验证
//   2. makeMoveFromDrag：给定"按在哪、拖到哪"，看它产生的 Move 合不合理
//
// 【它验证不了什么】
//   方向符号（dir 的正负）到底对不对 —— 那取决于你的坐标系约定，
//   必须用眼睛看。但这里能验证【自洽性】：
//   比如"在 +Z 面上往 +X 拖"和"往 +Y 拖"应该得到不同的轴，
//   而且转的层应该是同一个（都在前面那一层）。
//
// 用法：
//   cmake --build --preset debug --target drag_check
//   .\build\debug\drag_check.exe

#include "input/drag.hpp"

#include <cstdio>
#include <cmath>

#if defined(_WIN32)
#include <windows.h>
#endif

namespace {

int g_fail = 0;

void check(bool ok, const char* what) {
    if (!ok) {
        ++g_fail;
    }
    std::printf("  %s %s\n", ok ? "[ OK ]" : "[FAIL]", what);
}

const char* axisName(int a) {
    return a == 0 ? "X" : (a == 1 ? "Y" : "Z");
}

void dumpMove(const char* label, const cube::Move& m) {
    std::printf("     %-28s axis=%s(%d)  layer=%d  dir=%+d  turns=%d\n",
                label, axisName(m.axis), m.axis, m.layer, m.dir, m.turns);
}

// 造一条从 from 指向 to 的射线
cube::Ray rayBetween(const glm::vec3& from, const glm::vec3& to) {
    return {from, glm::normalize(to - from)};
}

}  // namespace

int main() {
#if defined(_WIN32)
    SetConsoleOutputCP(CP_UTF8);
#endif

    std::printf("drag_check —— 拖动到 Move 的映射验证\n\n");

    // =====================================================================
    // 一、intersectPlane
    // =====================================================================
    std::printf("[1] 射线与平面求交\n");
    {
        // 平面 z = 1.5（魔方前表面），法线 +Z
        const glm::vec3 planePoint(0.0f, 0.0f, 1.5f);
        const glm::vec3 planeNormal(0.0f, 0.0f, 1.0f);

        // 从 (0,0,7) 沿 -Z 射过去，应该打在 (0,0,1.5)
        glm::vec3 hit(0.0f);
        const bool ok = cube::intersectPlane(rayBetween({0,0,7}, {0,0,0}),
                                             planePoint, planeNormal, hit);
        std::printf("     打在 (%.3f, %.3f, %.3f)\n", hit.x, hit.y, hit.z);
        check(ok && std::fabs(hit.z - 1.5f) < 1e-3f &&
              std::fabs(hit.x) < 1e-3f && std::fabs(hit.y) < 1e-3f,
              "正对前表面，交于 (0,0,1.5)");

        // 斜着射：(3,3,7) 朝原点方向 → 求出和前表面的交点
        const glm::vec3 from(3.0f, 3.0f, 7.0f);
        const bool ok2 = cube::intersectPlane(rayBetween(from, glm::vec3(0.0f)),
                                              planePoint, planeNormal, hit);
        std::printf("     斜射交于 (%.3f, %.3f, %.3f)\n", hit.x, hit.y, hit.z);
        check(ok2 && std::fabs(hit.z - 1.5f) < 1e-3f, "斜射也能落在 z=1.5 上");

        // 平行于平面：从 (0,0,5) 朝 +X 射，永远打不到 z=1.5
        glm::vec3 dummy(0.0f);
        const bool ok3 = cube::intersectPlane(rayBetween({0,0,5}, {1,0,5}),
                                              planePoint, planeNormal, dummy);
        check(!ok3, "射线平行于平面 → 返回 false（而不是除零）");
    }

    // =====================================================================
    // 二、makeMoveFromDrag
    // =====================================================================
    std::printf("\n[2] 拖动映射（前表面 z=+1.5，法线 +Z）\n");
    {
        // 建立锚点：按在魔方前面的中心块上
        cube::PickResult pick;
        pick.hit = true;
        pick.point = glm::vec3(0.0f, 0.0f, 1.5f);
        pick.normal = glm::ivec3(0, 0, 1);
        pick.cell = glm::ivec3(0, 0, 1);

        const cube::DragAnchor anchor = cube::makeAnchor(pick);
        check(anchor.valid, "锚点建立成功");

        // --- 往 +X 拖（世界空间向右）---
        {
            const glm::vec3 to(0.6f, 0.0f, 1.5f);
            cube::Move m;
            const bool ok = cube::makeMoveFromDrag(
                anchor, rayBetween({0.0f, 0.0f, 7.0f}, to), m);
            if (ok) dumpMove("前表面 往 +X 拖", m);
            else std::printf("     前表面 往 +X 拖 → 没有产生动作\n");
            check(ok, "往 +X 拖产生了动作");
            if (ok) {
                check(m.axis != 2, "旋转轴【不是 Z】（因为拖的是面的切向）");
                check(m.turns == 1, "turns = 1");
                check(std::abs(m.dir) == 1, "dir = ±1");
            }
        }

        // --- 往 +Y 拖（世界空间向上）---
        {
            const glm::vec3 to(0.0f, 0.6f, 1.5f);
            cube::Move m;
            const bool ok = cube::makeMoveFromDrag(
                anchor, rayBetween({0.0f, 0.0f, 7.0f}, to), m);
            if (ok) dumpMove("前表面 往 +Y 拖", m);
            else std::printf("     前表面 往 +Y 拖 → 没有产生动作\n");
            check(ok, "往 +Y 拖产生了动作");
        }

        // --- 微动：应该被阈值挡掉 ---
        {
            const glm::vec3 to(0.02f, 0.0f, 1.5f);
            cube::Move m;
            const bool ok = cube::makeMoveFromDrag(
                anchor, rayBetween({0.0f, 0.0f, 7.0f}, to), m);
            check(!ok, "拖动太短 → 不产生动作（阈值生效）");
        }

        // --- 无效锚点 ---
        {
            const cube::DragAnchor bad;
            cube::Move m;
            const bool ok = cube::makeMoveFromDrag(
                bad, rayBetween({0.0f, 0.0f, 7.0f}, {0.6f, 0.0f, 1.5f}), m);
            check(!ok, "锚点无效 → 不产生动作");
        }
    }

    // =====================================================================
    // 三、不同面 → 旋转层应该不同
    // =====================================================================
    std::printf("\n[3] 拖不同的面，转的层应该跟着变\n");
    {
        struct Case {
            const char* name;
            glm::vec3 point;    // 按在哪
            glm::ivec3 normal;
            glm::ivec3 cell;
            glm::vec3 dragTo;   // 拖到哪
        };
        const Case cases[] = {
            {"前面中心往右拖",   {0.0f, 0.0f, 1.5f}, {0,0,1},  {0,0,1},  {0.6f, 0.0f, 1.5f}},
            {"前面中心往上拖",   {0.0f, 0.0f, 1.5f}, {0,0,1},  {0,0,1},  {0.0f, 0.6f, 1.5f}},
            {"上面中心往右拖",   {0.0f, 1.5f, 0.0f}, {0,1,0},  {0,1,0},  {0.6f, 1.5f, 0.0f}},
            {"右面中心往上拖",   {1.5f, 0.0f, 0.0f}, {1,0,0},  {1,0,0},  {1.5f, 0.6f, 0.0f}},
        };
        for (const Case& c : cases) {
            cube::PickResult pick;
            pick.hit = true;
            pick.point = c.point;
            pick.normal = c.normal;
            pick.cell = c.cell;
            const cube::DragAnchor a = cube::makeAnchor(pick);

            // 射线起点放在这个面外侧 5 个单位处
            const glm::vec3 n(static_cast<float>(c.normal.x),
                              static_cast<float>(c.normal.y),
                              static_cast<float>(c.normal.z));
            const glm::vec3 rayFrom = c.point + n * 5.0f;

            cube::Move m;
            const bool ok = cube::makeMoveFromDrag(a, rayBetween(rayFrom, c.dragTo), m);
            if (ok) {
                std::printf("     %-20s → axis=%s layer=%d dir=%+d\n",
                            c.name, axisName(m.axis), m.layer, m.dir);
                check(m.layer >= 0 && m.layer <= 2, "层号在 0..2 范围内");
            } else {
                std::printf("     %-20s → 未产生动作\n", c.name);
                check(false, c.name);
            }
        }
    }

    std::printf("\n");
    if (g_fail == 0) {
        std::printf("全部通过。注意：符号方向仍需用眼睛验证。\n");
    } else {
        std::printf("有 %d 项不对。\n", g_fail);
    }
    return g_fail == 0 ? 0 : 1;
}
