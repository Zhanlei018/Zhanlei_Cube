#include "input/picking.hpp"
#include <glm/gtc/matrix_transform.hpp>
#include <cstdio>
#include <cmath>

static int g_fail = 0;
static void check(bool ok, const char* what) {
    if (!ok) ++g_fail;
    std::printf("  %s %s\n", ok ? "[ OK ]" : "[FAIL]", what);
}

static glm::mat4 makeView() {
    return glm::lookAt(glm::vec3(0,0,7), glm::vec3(0.0f), glm::vec3(0,1,0));
}
static glm::mat4 makeProj() {
    return glm::perspective(glm::radians(45.0f), 800.0f/600.0f, 0.1f, 100.0f);
}

int main() {
    const int W = 800, H = 600;
    const glm::mat4 view = makeView();
    const glm::mat4 proj = makeProj();

    // ---------- 验证 1：射线构造 ----------
    std::printf("=== 验证 1：射线构造 ===\n");
    {
        cube::Ray r = cube::makeRayFromScreen(400, 300, W, H, view, proj);
        std::printf("     屏幕中心 → origin=(%.2f,%.2f,%.2f)  dir=(%.3f,%.3f,%.3f)\n",
                    r.origin.x, r.origin.y, r.origin.z,
                    r.direction.x, r.direction.y, r.direction.z);
        check(std::fabs(r.origin.x) < 0.01f && std::fabs(r.origin.y) < 0.01f &&
              std::fabs(r.origin.z - 7.0f) < 0.01f, "起点 ≈ (0,0,7)");
        check(r.direction.x > -0.01f && r.direction.x < 0.01f &&
              r.direction.y > -0.01f && r.direction.y < 0.01f &&
              r.direction.z < -0.99f, "方向 ≈ (0,0,-1)");
    }
    {
        struct { int x, y; float sx, sy; const char* name; } pts[] = {
            {0,   0,   -1,  1, "左上"},
            {W,   0,    1,  1, "右上"},
            {0,   H,   -1, -1, "左下"},
            {W,   H,    1, -1, "右下"},
        };
        for (auto& p : pts) {
            cube::Ray r = cube::makeRayFromScreen(p.x, p.y, W, H, view, proj);
            bool okx = (p.sx > 0) ? (r.direction.x > 0.05f) : (r.direction.x < -0.05f);
            bool oky = (p.sy > 0) ? (r.direction.y > 0.05f) : (r.direction.y < -0.05f);
            char buf[96];
            std::snprintf(buf, sizeof(buf), "%s 方向 x%s y%s (实际 %.2f, %.2f)", p.name,
                          p.sx > 0 ? ">0" : "<0", p.sy > 0 ? ">0" : "<0",
                          r.direction.x, r.direction.y);
            check(okx && oky, buf);
        }
    }

    // ---------- 验证 2：求交 ----------
    std::printf("\n=== 验证 2：屏幕中心射线的交点 ===\n");
    {
        cube::PickResult p = cube::pickAtScreen(400, 300, W, H, view, proj);
        std::printf("     hit=%d  point=(%.3f,%.3f,%.3f)  normal=(%d,%d,%d)  cell=(%d,%d,%d)\n",
                    p.hit ? 1 : 0, p.point.x, p.point.y, p.point.z,
                    p.normal.x, p.normal.y, p.normal.z,
                    p.cell.x, p.cell.y, p.cell.z);
        check(p.hit, "命中");
        check(std::fabs(p.point.z - 1.5f) < 0.01f, "交点 z ≈ 1.5（前面）");
        check(p.normal == glm::ivec3(0,0,1), "法线 = (0,0,1) 朝外");
        check(p.cell == glm::ivec3(0,0,1), "格子 = (0,0,1)");
    }

    // ---------- 验证 3：往返一致性 ----------
    std::printf("\n=== 验证 3：往返一致性（正投影回屏幕 → 再拾取回来）===\n");
    {
        glm::vec3 tests[] = {
            {0.0f, 0.0f, 1.5f},
            {1.0f, 0.5f, 1.5f},
            {-0.7f, -1.0f, 1.5f},
        };
        for (auto& w : tests) {
            glm::vec4 clip = proj * view * glm::vec4(w, 1.0f);
            glm::vec3 ndc = glm::vec3(clip) / clip.w;
            int sx = (int)((ndc.x + 1.0f) * 0.5f * W);
            int sy = (int)((1.0f - ndc.y) * 0.5f * H);
            cube::PickResult p = cube::pickAtScreen(sx, sy, W, H, view, proj);
            float err = p.hit ? glm::length(p.point - w) : 999.0f;
            char buf[128];
            std::snprintf(buf, sizeof(buf),
                "世界(%.1f,%.1f,%.1f) → 像素(%d,%d) → 拾取(%.2f,%.2f,%.2f) 误差 %.3f",
                w.x, w.y, w.z, sx, sy, p.point.x, p.point.y, p.point.z, err);
            check(p.hit && err < 0.05f, buf);
        }
    }

    // ---------- 补充：屏幕角落应该打不到魔方 ----------
    std::printf("\n=== 补充：角落应该打不到 ===\n");
    {
        cube::PickResult p = cube::pickAtScreen(2, 2, W, H, view, proj);
        check(!p.hit, "屏幕左上角未命中魔方");
    }

    std::printf("\n%s（失败 %d 项）\n", g_fail == 0 ? "全部通过" : "有失败", g_fail);
    return g_fail == 0 ? 0 : 1;
}
