// tests/test_main.cpp —— 极简测试框架
//
// 故意不引入 gtest / Catch2：这个项目需要的是"30 行能跑"，不是"配半小时依赖"。
// 用法见本目录 README.md。
//
// 你接下来要做的：把下面的 EXPECT 宏按需补全（目前只有骨架）。

#include "test_framework.hpp"

#include <cstdio>

namespace cubetest {

int g_checks = 0;
int g_failures = 0;
const char* g_current_case = "<none>";

void beginCase(const char* name) {
    g_current_case = name;
    std::printf("[ RUN      ] %s\n", name);
}

void endCase() {
    std::printf("[       OK ] %s\n", g_current_case);
}

int summary() {
    std::printf("\n================================\n");
    if (g_failures == 0) {
        std::printf(" 全部通过：%d 项检查\n", g_checks);
    } else {
        std::printf(" 失败 %d / %d 项检查\n", g_failures, g_checks);
    }
    std::printf("================================\n");
    return g_failures == 0 ? 0 : 1;
}

}  // namespace cubetest

// 各个测试文件的入口，在这里显式调用。
// 想加新测试：在 tests/ 建 .cpp，写一个 runXxxTests()，然后加进下面这张表。
void runCubeTests();

int main() {
    std::printf("cube_tests —— 纯逻辑测试（不涉及 OpenGL）\n\n");

    runCubeTests();

    return cubetest::summary();
}
