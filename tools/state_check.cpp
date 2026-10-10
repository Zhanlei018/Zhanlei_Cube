// tools/state_check.cpp —— 只验证 state.cpp（不依赖 moves.cpp）
//
// 临时验证器：等 moves.cpp 写完并跑通 cube_check 之后可以删掉。
//
// 用法：
//   cmake --build --preset debug --target state_check
//   .\build\debug\state_check.exe

#include "cube/state.hpp"

#include <cstdio>

#if defined(_WIN32)
#include <windows.h>
#endif

namespace {

int g_failed = 0;

void expect(bool ok, const char* what) {
    if (!ok) {
        ++g_failed;
    }
    std::printf("  %s %s\n", ok ? "[ OK ]" : "[FAIL]", what);
}

}  // namespace

int main() {
#if defined(_WIN32)
    SetConsoleOutputCP(CP_UTF8);
#endif

    std::printf("state_check —— 只验证 state.cpp\n\n");

    cube::State solved = cube::makeSolvedState();

    // ---- 1. 初始状态 ----
    std::printf("[1] 初始状态\n");
    expect(solved.cubies.size() == 26, "共 26 个小方块");
    expect(solved.history.empty(), "history 初始为空");
    expect(cube::checkInvariants(solved), "checkInvariants() 通过");

    // ---- 2. 六个面各自同色（并报告每个面是什么颜色）----
    std::printf("\n[2] 复原态下，六个面应各自同色\n");
    for (char face : {'U', 'D', 'L', 'R', 'F', 'B'}) {
        const auto colors = cube::faceColors(solved, face);
        bool uniform = true;
        for (cube::Color c : colors) {
            if (c != colors[0]) {
                uniform = false;
            }
        }
        bool allKnown = (colors[0] != cube::Color::kUnknown);

        char label[80];
        std::snprintf(label, sizeof(label), "面 %c 九格同色，颜色 = %c",
                      face, cube::colorChar(colors[0]));
        expect(uniform && allKnown, label);
    }

    // ---- 3. 颜色约定对不对（+X 红 / +Y 白 / +Z 绿 ...）----
    std::printf("\n[3] 颜色约定（对照 NOTES.md 的表）\n");
    struct Expect {
        char face;
        cube::Color want;
        const char* desc;
    };
    const Expect expectations[] = {
        {'R', cube::Color::kRed,    "R 面(+X) 应为红"},
        {'L', cube::Color::kOrange, "L 面(-X) 应为橙"},
        {'U', cube::Color::kWhite,  "U 面(+Y) 应为白"},
        {'D', cube::Color::kYellow, "D 面(-Y) 应为黄"},
        {'F', cube::Color::kGreen,  "F 面(+Z) 应为绿"},
        {'B', cube::Color::kBlue,   "B 面(-Z) 应为蓝"},
    };
    for (const Expect& e : expectations) {
        const auto colors = cube::faceColors(solved, e.face);
        expect(colors[0] == e.want, e.desc);
    }

    // ---- 4. isSolved ----
    std::printf("\n[4] isSolved\n");
    expect(cube::isSolved(solved), "复原态 isSolved() 为真");

    // 手动破坏一个贴纸的朝向，isSolved 必须变假
    {
        cube::State broken = cube::makeSolvedState();
        broken.cubies[0].orient = glm::imat3(0);   // 行列式为 0 的垃圾矩阵
        expect(!cube::isSolved(broken), "破坏一个方块后 isSolved() 为假");
    }

    // ---- 5. operator== ----
    std::printf("\n[5] operator==\n");
    {
        cube::State a = cube::makeSolvedState();
        cube::State b = cube::makeSolvedState();
        expect(a == b, "两个初始状态相等");
        b.cubies[5].pos = glm::ivec3(0, 0, 0);
        expect(a != b, "改一个方块后不相等");
    }

    // ---- 6. 非法面参数 ----
    std::printf("\n[6] 非法面参数\n");
    {
        const auto bad = cube::faceColors(solved, 'X');
        expect(bad[0] == cube::Color::kUnknown, "faceColors(state, 'X') 返回 kUnknown");
    }

    // ---- 7. 打印出来肉眼看 ----
    std::printf("\n[7] 复原态展开图（六面应各自同色，且颜色对应正确）\n");
    cube::printState(solved);

    std::printf("\n");
    if (g_failed == 0) {
        std::printf("全部通过 —— state.cpp 成立。\n");
    } else {
        std::printf("有 %d 项不对。\n", g_failed);
    }
    return g_failed == 0 ? 0 : 1;
}
