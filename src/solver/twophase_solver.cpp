#include "solver/twophase_solver.hpp"

#include <array>
#include <cstdio>
#include <sstream>

#include <min2phase/min2phase.h>

#include "cube/moves.hpp"

namespace cube {

namespace {

// ---------------------------------------------------------------------------
//  facelet 串：54 个字符，顺序 U(9) R(9) F(9) D(9) L(9) B(9)
//
//  每面 9 个贴纸按【从上到下、从左到右】排列 —— 正好就是 faceColors()
//  返回的 row*3+col 顺序（那个函数本来就是给 printState 排版用的）。
//
//  颜色字符（min2phase 约定）：U=白 R=红 F=绿 D=黄 L=橙 B=蓝，
//  和本项目的 colorOfSticker 一一对应，直接用即可。
// ---------------------------------------------------------------------------
char colorToFacelet(Color c) {
    switch (c) {
        case Color::kWhite:  return 'U';
        case Color::kRed:    return 'R';
        case Color::kGreen:  return 'F';
        case Color::kYellow: return 'D';
        case Color::kOrange: return 'L';
        case Color::kBlue:   return 'B';
        default:             return '?';
    }
}

// ---------------------------------------------------------------------------
//  ⚠️⚠️ 面字母 → Move.dir：这里是整个集成最容易错的地方，务必看清 ⚠️⚠️
//
//  min2phase 用【标准魔方记号】（R = 从 +X 方向看的顺时针 90°）。
//  本引擎的 Move.dir 在【正坐标层】和【负坐标层】上方向不一致。
//
//  实测（做法：盯着一个方块看它转去哪）：
//      面   引擎 dir=+1 的旋转      标准记号里对应
//      R     U→F→D→B→U             R'（逆时针）
//      L     U→F→D→B→U             L（顺时针）
//      U     B→L→F→R→B             U'
//      D     B→L→F→R→B             D
//      F     B→U→R→D→B             F'
//      B     B→U→R→D→B             B
//
//  结论：layer == 2（R/U/F）要用 dir = -1，layer == 0（L/D/B）用 dir = +1。
//  取反之后 6 个面才统一：引擎 dir 的含义 = "标准记号的顺时针"。
//
//  ⚠️ 为什么以前没发现：R2 / L2 这类 180° 动作是自逆的，方向错了也看不出。
//     必须用 90° 的动作做"打乱 → 求解 → 回代"闭环才能暴露。
//     守门员见 tools/twophase_check.cpp 的逐动作闭环那段。
// ---------------------------------------------------------------------------
bool faceLetterToMove(char letter, Move& out) {
    switch (letter) {
        case 'R': out.axis = kAxisX; out.layer = 2; break;
        case 'L': out.axis = kAxisX; out.layer = 0; break;
        case 'U': out.axis = kAxisY; out.layer = 2; break;
        case 'D': out.axis = kAxisY; out.layer = 0; break;
        case 'F': out.axis = kAxisZ; out.layer = 2; break;
        case 'B': out.axis = kAxisZ; out.layer = 0; break;
        default:  return false;
    }
    out.turns = 1;
    out.dir = (out.layer == 2) ? -1 : 1;   // 正坐标层的方向是反的
    return true;
}

}  // namespace

std::string toFacelets(const State& s) {
    static const char kFaceOrder[6] = {'U', 'R', 'F', 'D', 'L', 'B'};
    std::string out;
    out.reserve(54);
    for (char face : kFaceOrder) {
        const std::array<Color, 9> colors = faceColors(s, face);
        for (Color c : colors) out.push_back(colorToFacelet(c));
    }
    return out;
}

void warmUpTwoPhase() {
    static bool done = false;
    if (done) return;
    min2phase::init();
    done = true;
}

std::vector<Move> solveTwoPhase(const State& s) {
    warmUpTwoPhase();

    const std::string facelets = toFacelets(s);
    if (facelets.size() != 54) return {};

    // maxDepth=21：21 步内几乎总能找到解
    // probeMin=0 表示不强制多探索，优先速度
    uint8_t usedMoves = 0;
    const std::string solution = min2phase::solve(facelets, 21, 1000000, 0, 0, &usedMoves);
    if (solution.empty()) return {};

    // 解析 "R2 U' F ..." 这种字符串
    std::vector<Move> moves;
    std::istringstream iss(solution);
    std::string token;
    while (iss >> token) {
        if (token.empty() || token[0] == '.') continue;  // "." 是阶段分隔符
        Move m{};
        if (!faceLetterToMove(token[0], m)) continue;
        if (token.size() >= 2) {
            if (token[1] == '\'') {
                m.dir = -m.dir;   // 逆时针 = 基准方向取反
            } else if (token[1] == '2') {
                m.turns = 2;      // 180°：方向无意义
            }
        }
        moves.push_back(m);
    }
    return moves;
}

}  // namespace cube
