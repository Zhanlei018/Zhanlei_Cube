#pragma once

#include <string>
#include <vector>

#include "cube/state.hpp"

namespace cube {

// ===========================================================================
//  Kociemba 两阶段求解器（封装第三方库 min2phase）
//
//  用法：
//      warmUpTwoPhase();                          // 建表，约 55 ms，只需一次
//      std::vector<Move> sol = solveTwoPhase(s);  // 之后每次约 0.5 ms
//
//  和 Thistlethwaite（四阶段）的区别：
//      四阶段每阶段都要建一张覆盖整个坐标空间的距离表，Debug 下要几十秒；
//      这个库靠对称性归约把剪枝表压到约 1 MB，建表 55 ms、求解 0.5 ms。
//
//  它是两阶段：先进入 G1 = ⟨U,D,R2,L2,F2,B2⟩，再在 G1 里解完。
//  因为它会不断尝试新的阶段一路线，解通常在 20~21 步。
// ===========================================================================

// 建表（幂等；第一次调用约 55 ms，之后立即返回）
void warmUpTwoPhase();

// 求解。返回一串动作；s 不会被修改。
// 失败（比如状态非法）返回空 vector。
std::vector<Move> solveTwoPhase(const State& s);

// 把状态转成 min2phase 要的 54 字符 facelet 串（URFDLB 顺序）。
// 单独导出是为了方便测试台做对照。
std::string toFacelets(const State& s);

}  // namespace cube
