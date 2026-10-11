#include "input/drag.hpp"
#include <cstdio>
#include <cmath>

static int g_fail = 0;
static void check(bool ok, const char* what) {
    if (!ok) ++g_fail;
    std::printf("  %s %s\n", ok ? "[ OK ]" : "[FAIL]", what);
}

static cube::Ray rayFrom(const glm::vec3& surfacePoint, const glm::vec3& n, const glm::vec3& dragTo) {
    return { surfacePoint + n * 5.0f, glm::normalize(dragTo - (surfacePoint + n * 5.0f)) };
}

int main() {
    struct Case {
        const char* name;
        glm::vec3 point; glm::vec3 normal; glm::ivec3 cell;
        glm::vec3 dragTo;
        int expAxis;      // 期望旋转轴
        int expLayer;     // 期望层号（精确）
    };
    const Case cases[] = {
        // 在【正面中心】往右拖：抓住中层方块往右推 → 转【Y 轴中层】(layerOf(0)=1)
        {"正面中心 往右拖", {0,0,1.5f}, {0,0,1}, {0,0,1},   {0.6f,0,1.5f}, 1, 1},
        // 在【正面顶部】往右拖：cell.y=+1 → layerOf(1)=2 → 顶层
        {"正面顶部 往右拖", {0,1.0f,1.5f}, {0,0,1}, {0,1,1}, {0.6f,1.0f,1.5f}, 1, 2},
        // 在【正面底部】往右拖：cell.y=-1 → layerOf(-1)=0 → 底层
        {"正面底部 往右拖", {0,-1.0f,1.5f}, {0,0,1}, {0,-1,1}, {0.6f,-1.0f,1.5f}, 1, 0},
        // 在【正面中央】往上拖：cell.x=0 → layerOf(0)=1 → 中层（绕 X）
        {"正面中心 往上拖", {0,0,1.5f}, {0,0,1}, {0,0,1},   {0,0.6f,1.5f}, 0, 1},
        // 在【正面右侧】往上拖：cell.x=+1 → layerOf(1)=2 → 右层
        {"正面右侧 往上拖", {1.0f,0,1.5f}, {0,0,1}, {1,0,1}, {1.0f,0.6f,1.5f}, 0, 2},
    };

    std::printf("=== 精确层号验证 ===\n");
    for (const Case& c : cases) {
        cube::PickResult pick;
        pick.hit = true; pick.point = c.point; pick.normal = glm::ivec3(c.normal);
        pick.cell = c.cell;
        cube::DragAnchor a = cube::makeAnchor(pick);
        cube::Move m;
        bool ok = cube::makeMoveFromDrag(a, rayFrom(c.point, c.normal, c.dragTo), m);
        char buf[160];
        if (!ok) {
            std::snprintf(buf, sizeof(buf), "%s → 没产生动作", c.name);
            check(false, buf);
            continue;
        }
        std::snprintf(buf, sizeof(buf), "%-16s axis=%d(期望%d) layer=%d(期望%d) dir=%+d",
                      c.name, m.axis, c.expAxis, m.layer, c.expLayer, m.dir);
        check(m.axis == c.expAxis && m.layer == c.expLayer, buf);
    }

    std::printf("\n=== 六个面的方向符号（供你判断正反）===\n");
    std::printf("  观察方法：假设你就是抓着那个方块，往指定方向推。\n");
    std::printf("  把下面每个 dir 记下来，然后到程序里实测；如果【全部相反】，就把 dir 整体取反。\n\n");
    struct F { const char* n; glm::vec3 p; glm::vec3 nm; glm::ivec3 c; glm::vec3 d; };
    const F faces[] = {
        {"前面(+Z) 往 +X 拖", {0,0,1.5f},  {0,0,1},  {0,0,1}, {0.6f,0,1.5f}},
        {"前面(+Z) 往 +Y 拖", {0,0,1.5f},  {0,0,1},  {0,0,1}, {0,0.6f,1.5f}},
        {"上面(+Y) 往 +X 拖", {0,1.5f,0},  {0,1,0},  {0,1,0}, {0.6f,1.5f,0}},
        {"右面(+X) 往 +Y 拖", {1.5f,0,0},  {1,0,0},  {1,0,0}, {1.5f,0.6f,0}},
    };
    for (const F& f : faces) {
        cube::PickResult pick;
        pick.hit = true; pick.point = f.p; pick.normal = glm::ivec3(f.nm); pick.cell = f.c;
        cube::DragAnchor a = cube::makeAnchor(pick);
        cube::Move m;
        if (cube::makeMoveFromDrag(a, rayFrom(f.p, f.nm, f.d), m)) {
            std::printf("  %-18s → axis=%d layer=%d dir=%+d\n", f.n, m.axis, m.layer, m.dir);
        } else {
            std::printf("  %-18s → 无动作\n", f.n);
        }
    }

    std::printf("\n%s（失败 %d 项）\n", g_fail == 0 ? "全部通过" : "有失败", g_fail);
    return g_fail == 0 ? 0 : 1;
}
