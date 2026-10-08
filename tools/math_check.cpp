// tools/math_check.cpp —— 数学层验证台
//
// 为什么单独放一个文件？
//   因为一个可执行文件里只能有一个 main()。
//   math.cpp 是要被链接进 cube_core 库的，它不能有 main()；
//   验证用的 main() 就放在这里，通过 CMake 单独编成一个程序。
//
// 用法：
//   cmake --build --preset debug
//   .\build\debug\math_check.exe

#include "math/math.hpp"

#include <cstdio>
#include <glm/vec3.hpp>

namespace {

int g_failed = 0;

void report(const char* label, int actualX, int actualY, int actualZ,
            int wantX, int wantY, int wantZ) {
    const bool ok = (actualX == wantX && actualY == wantY && actualZ == wantZ);
    if (!ok) {
        ++g_failed;
    }
    std::printf("  %s %-34s 得到 (%2d,%2d,%2d)   期望 (%2d,%2d,%2d)\n",
                ok ? "[ OK ]" : "[FAIL]", label,
                actualX, actualY, actualZ, wantX, wantY, wantZ);
}

}  // namespace

int main() {
    std::printf("math_check —— 验证整数旋转矩阵\n\n");

    // ---- 用例 1：绕 X 轴转 90°，+Y 应该落到 +Z -----------------------------
    {
        const glm::ivec3 v(0, 1, 0);
        const glm::ivec3 r = cube::rotX(1) * v;
        report("rotX(1) * (0,1,0)", r.x, r.y, r.z, 0, 0, 1);
    }

    // ---- 用例 2：绕 X 轴转 90°，+Z 应该落到 -Y -----------------------------
    {
        const glm::ivec3 v(0, 0, 1);
        const glm::ivec3 r = cube::rotX(1) * v;
        report("rotX(1) * (0,0,1)", r.x, r.y, r.z, 0, -1, 0);
    }

    // ---- 用例 3：转 4 次必须回到原处（最能抓出符号写反）--------------------
    {
        glm::ivec3 r(0, 1, 0);
        for (int i = 0; i < 4; ++i) {
            r = cube::rotX(1) * r;
        }
        report("rotX(1) 连做 4 次", r.x, r.y, r.z, 0, 1, 0);
    }

    // ---- 用例 4：180° 就是转两次 90°，两条路线结果必须一致 -----------------
    {
        const glm::ivec3 v(0, 1, 0);
        const glm::ivec3 once = cube::rotX(2) * v;
        const glm::ivec3 twice = cube::rotX(1) * (cube::rotX(1) * v);
        report("rotX(2) 对比连做两次 rotX(1)",
               once.x, once.y, once.z, twice.x, twice.y, twice.z);
    }

    // ---- 用例 5：layerOf -----------------------------------------------------
    {
        const int a = cube::layerOf(-1);
        const int b = cube::layerOf(0);
        const int c = cube::layerOf(1);
        const bool ok = (a == 0 && b == 1 && c == 2);
        if (!ok) {
            ++g_failed;
        }
        std::printf("  %s %-34s 得到 (%d,%d,%d)     期望 (0,1,2)\n",
                    ok ? "[ OK ]" : "[FAIL]", "layerOf(-1/0/1)", a, b, c);
    }

    // ---- 用例 6：rotY / rotZ 也顺手验一下 ----------------------------------
    {
        const glm::ivec3 v(0, 0, 1);
        const glm::ivec3 r = cube::rotY(1) * v;   // 绕 Y 转：+Z 应该落到 +X
        report("rotY(1) * (0,0,1)", r.x, r.y, r.z, 1, 0, 0);
    }
    {
        const glm::ivec3 v(1, 0, 0);
        const glm::ivec3 r = cube::rotZ(1) * v;   // 绕 Z 转：+X 应该落到 +Y
        report("rotZ(1) * (1,0,0)", r.x, r.y, r.z, 0, 1, 0);
    }

    std::printf("\n");
    if (g_failed == 0) {
        std::printf("全部通过 —— 矩阵是对的，可以开始写魔方了。\n");
    } else {
        std::printf("有 %d 项不对。\n", g_failed);
        std::printf("提示：检查矩阵是不是按\"列\"填的，以及右手定则的方向。\n");
    }
    return g_failed == 0 ? 0 : 1;
}
