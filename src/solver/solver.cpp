#include "solver/solver.hpp"

#include <random>

namespace cube {
std::vector<Move> randomScramble(int steps, std::uint32_t seed) {
    // ⚠️ 打乱器【只能】产生 6 个外层面的转动。
    //
    // Move.layer 的语义是"切片号"：0 → 坐标 -1，1 → 坐标 0，2 → 坐标 +1
    // （applyMove 的筛选条件是 pos[axis] == layer - 1）。
    // 其中 layer == 1 是【中层切片】，它在纯面转模型里不是合法动作：
    // 只循环 4 个中层棱块、一个角块都不动，于是得到"棱块置换奇、角块置换偶"
    // 的状态 —— 这种状态在真实魔方上不存在。
    //
    // 原来的写法随机取 layer ∈ {0,1,2}，实测 25 步里平均有 8.1 步是中层转，
    // 20 个打乱样本里只有 8 个是合法魔方。用这种状态喂求解器必然算错。
    //
    // 所以改成按【面】抽取，(axis, layer) 成对出现，layer 永远是 0 或 2。
    static const int kAxis[6] = {0, 0, 1, 1, 2, 2};    // R L U D F B
    static const int kLayer[6] = {2, 0, 2, 0, 2, 0};

    std::vector<Move> sequence;
    sequence.reserve(steps);
    std::mt19937 gen(seed);
    std::uniform_int_distribution<> face_dist(0, 5);
    std::uniform_int_distribution<> dir_dist(0, 1);
    std::uniform_int_distribution<> turns_dist(1, 2);

    int last_face = -1;
    int second_last_face = -1;
    while (static_cast<int>(sequence.size()) < steps) {
        const int face = face_dist(gen);
        // 同一面连续转两次可以合并，属于冗余
        if (face == last_face) {
            continue;
        }
        // 三个不同面而首尾同面时同样可以合并，跳过这种组合
        if (face == second_last_face && last_face != -1) {
            continue;
        }

        Move m;
        m.axis = kAxis[face];
        m.layer = kLayer[face];
        m.turns = turns_dist(gen);
        if (m.turns == 2) {
            m.dir = 1;
        } else {
            m.dir = (dir_dist(gen) == 0) ? 1 : -1;
        }
        sequence.push_back(m);
        second_last_face = last_face;
        last_face = face;
    }
    return sequence;
}
}  // namespace cube
