// tools/scramble_check.cpp —— 打乱序列的合法性验证
//
// 用法：
//   cmake --build --preset debug --target scramble_check
//   .\build\debug\scramble_check.exe

#include "cube/moves.hpp"
#include "solver/solver.hpp"

#include <cstdio>
#include <set>

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

int layerId(const cube::Move& m) {
    return m.axis * 3 + m.layer;
}

}  // namespace

int main() {
#if defined(_WIN32)
    SetConsoleOutputCP(CP_UTF8);
#endif

    std::printf("scramble_check —— 打乱序列验证\n\n");

    const auto moves = cube::randomScramble(25);

    std::printf("[1] 基本形状\n");
    {
        char buf[64];
        std::snprintf(buf, sizeof(buf), "步数 = %zu（期望 25）", moves.size());
        check(moves.size() == 25, buf);
    }

    if (moves.empty()) {
        std::printf("\n  序列是空的 —— 还没实现？\n");
        return 1;
    }

    std::printf("\n[2] 每个动作都合法\n");
    {
        bool ok = true;
        for (const cube::Move& m : moves) {
            if (m.axis < 0 || m.axis > 2) ok = false;
            if (m.layer < 0 || m.layer > 2) ok = false;
            if (m.dir != 1 && m.dir != -1) ok = false;
            if (m.turns < 1 || m.turns > 2) ok = false;
        }
        check(ok, "axis/layer/dir/turns 全在合法范围");
    }

    std::printf("\n[3] 约束：相邻两步和相隔一步的两步都不同层\n");
    {
        bool ok = true;
        int badAt = -1;
        for (std::size_t i = 0; i + 1 < moves.size(); ++i) {
            if (layerId(moves[i]) == layerId(moves[i + 1])) {
                ok = false;
                badAt = static_cast<int>(i);
                break;
            }
        }
        for (std::size_t i = 0; i + 2 < moves.size() && ok; ++i) {
            if (layerId(moves[i]) == layerId(moves[i + 2])) {
                ok = false;
                badAt = static_cast<int>(i);
                break;
            }
        }
        char buf[96];
        if (ok) {
            std::snprintf(buf, sizeof(buf), "没有重复层（也检查了 R L R 这种模式）");
        } else {
            std::snprintf(buf, sizeof(buf), "第 %d 步附近有重复层", badAt);
        }
        check(ok, buf);
    }

    std::printf("\n[4] 同一个种子必须产生同一个序列（可复现）\n");
    {
        const auto a = cube::randomScramble(25, 12345u);
        const auto b = cube::randomScramble(25, 12345u);
        bool same = (a.size() == b.size());
        for (std::size_t i = 0; same && i < a.size(); ++i) {
            same = (a[i].axis == b[i].axis) && (a[i].layer == b[i].layer) &&
                   (a[i].dir == b[i].dir) && (a[i].turns == b[i].turns);
        }
        check(same, "种子 12345 两次结果一致");

        const auto c = cube::randomScramble(25, 999u);
        bool differ = (c.size() != a.size());
        for (std::size_t i = 0; !differ && i < a.size(); ++i) {
            if (a[i].axis != c[i].axis || a[i].layer != c[i].layer ||
                a[i].dir != c[i].dir || a[i].turns != c[i].turns) {
                differ = true;
            }
        }
        check(differ, "不同种子产生不同序列");
    }

    std::printf("\n[5] 用到的层足够分散（不是只有几个层在转）\n");
    {
        std::set<int> used;
        for (const cube::Move& m : moves) used.insert(layerId(m));
        char buf[96];
        std::snprintf(buf, sizeof(buf), "用到了 %zu 个不同的层（共 9 个）", used.size());
        check(used.size() >= 5, buf);
    }

    std::printf("\n[6] 打乱后确实乱了（转 25 步再检查 isSolved）\n");
    {
        cube::State s = cube::makeSolvedState();
        for (const cube::Move& m : moves) {
            cube::applyMove(s, m);
        }
        check(!cube::isSolved(s), "isSolved() == false");

        const auto undo = cube::invertedSequence(moves);
        for (const cube::Move& m : undo) {
            cube::applyMove(s, m);
        }
        check(cube::isSolved(s), "逆序复原后 isSolved() == true");
    }

    std::printf("\n=== 序列内容（前 10 步）===\n  ");
    for (std::size_t i = 0; i < moves.size() && i < 10; ++i) {
        std::printf("(%d,%d,%+d,%d) ", moves[i].axis, moves[i].layer, moves[i].dir, moves[i].turns);
    }
    std::printf("\n");

    std::printf("\n%s（失败 %d 项）\n", g_fail == 0 ? "全部通过" : "有失败", g_fail);
    return g_fail == 0 ? 0 : 1;
}
