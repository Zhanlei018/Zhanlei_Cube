// tools/cube_check.cpp —— 逻辑层验证台（阶段 1 的第二块）
//
// 跟 math_check 一样的分工：这里只有 main()，实现从 cube_core 里链接。
//
// 用法（等 src/cube/*.cpp 写完之后）：
//   cmake --build --preset debug
//   .\build\debug\cube_check.exe

#include "cube/moves.hpp"
#include "cube/state.hpp"

#include <cstdio>
#include <cstring>

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

// 检查状态不变量：|pos|<=1、orient 行列式=+1、26 个 pos 互不重复
void checkState(const char* label, const cube::State& s) {
    expect(cube::checkInvariants(s), label);
}

}  // namespace

int main() {
#if defined(_WIN32)
    SetConsoleOutputCP(CP_UTF8);
#endif

    std::printf("cube_check —— 验证魔方逻辑层\n\n");

    // ---------------------------------------------------------------------
    // 1. 初始状态
    // ---------------------------------------------------------------------
    std::printf("[1] 初始状态\n");
    cube::State solved = cube::makeSolvedState();

    expect(solved.cubies.size() == 26, "共 26 个小方块");
    expect(cube::isSolved(solved), "isSolved() 为真");
    checkState("checkInvariants() 通过", solved);

    // ---------------------------------------------------------------------
    // 2. 六个面各自同色
    // ---------------------------------------------------------------------
    std::printf("\n[2] 复原态下，六个面应该各自同色\n");
    for (char face : {'U', 'D', 'L', 'R', 'F', 'B'}) {
        const auto colors = cube::faceColors(solved, face);
        bool uniform = true;
        for (cube::Color c : colors) {
            if (c != colors[0]) {
                uniform = false;
            }
        }
        char label[64];
        std::snprintf(label, sizeof(label), "面 %c 九格同色（颜色 %c）",
                      face, cube::colorChar(colors[0]));
        expect(uniform, label);
    }

    // ---------------------------------------------------------------------
    // 3. 画出来看
    // ---------------------------------------------------------------------
    std::printf("\n[3] 打印初始状态（肉眼看这六个面是否各自同色）\n\n");
    cube::printState(solved);

    // ---------------------------------------------------------------------
    // 4. 单个 Move 转 4 次回到原状
    // ---------------------------------------------------------------------
    std::printf("\n[4] 转动测试\n");
    {
        const auto parsed = cube::parseMove("U");
        if (!parsed.has_value()) {
            expect(false, "parseMove(\"U\") 解析成功");
        } else {
            expect(true, "parseMove(\"U\") 解析成功");

            cube::State s = cube::makeSolvedState();
            cube::Move m = *parsed;
            m.turns = 1;
            for (int i = 0; i < 4; ++i) {
                cube::applyMove(s, m);
            }
            expect(s == solved, "U 转 4 次 == 初始状态");
            checkState("转 4 次后不变量仍成立", s);
        }
    }

    // ---------------------------------------------------------------------
    // 5. 180° == 转两次 90°
    // ---------------------------------------------------------------------
    {
        const auto p = cube::parseMove("U2");
        if (p.has_value()) {
            cube::State a = cube::makeSolvedState();
            cube::applyMove(a, *p);

            cube::State b = cube::makeSolvedState();
            cube::Move once = *p;
            once.turns = 1;
            cube::applyMove(b, once);
            cube::applyMove(b, once);

            expect(a == b, "U2 == U 连做两次");
        }
    }

    // ---------------------------------------------------------------------
    // 6. R U R' U' 做 6 次回到原状（周期测试，专抓朝向 bug）
    // ---------------------------------------------------------------------
    {
        const auto seq = cube::parseSequence("R U R' U'");
        if (seq.size() == 4) {
            cube::State s = cube::makeSolvedState();
            for (int round = 0; round < 6; ++round) {
                for (const cube::Move& m : seq) {
                    cube::applyMove(s, m);
                }
            }
            expect(s == solved, "R U R' U' 做 6 次 == 初始状态");
        } else {
            expect(false, "parseSequence(\"R U R' U'\") 解析出 4 个动作");
        }
    }

    // ---------------------------------------------------------------------
    // 7. 逆序复原
    // ---------------------------------------------------------------------
    {
        const auto seq = cube::parseSequence("R U F' L2 D");
        cube::State s = cube::makeSolvedState();
        for (const cube::Move& m : seq) {
            cube::applyMove(s, m);
        }
        expect(!cube::isSolved(s), "打乱后 isSolved() 为假");

        const auto undo = cube::invertedSequence(seq);
        for (const cube::Move& m : undo) {
            cube::applyMove(s, m);
        }
        expect(s == solved, "逆序复原后 == 初始状态");
    }

    std::printf("\n");
    if (g_failed == 0) {
        std::printf("全部通过 —— 逻辑层成立。\n");
    } else {
        std::printf("有 %d 项不对。\n", g_failed);
    }
    return g_failed == 0 ? 0 : 1;
}
