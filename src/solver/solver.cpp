#include "solver/solver.hpp"

#include <random>

namespace cube {
std::vector<Move> randomScramble(int steps, std::uint32_t seed) {
    std::vector<Move> sequence;
    sequence.reserve(steps);
    std::mt19937 gen(seed);
    std::uniform_int_distribution<> axis_dist(0, 2);
    std::uniform_int_distribution<> layer_dist(0, 2);
    std::uniform_int_distribution<> dir_dist(0, 1);
    std::uniform_int_distribution<> turns_dist(1, 2);
    while (static_cast<int>(sequence.size()) < steps) {
        int axis = axis_dist(gen);
        int layer = layer_dist(gen);
        int current_id = axis * 3 + layer;
        if (!sequence.empty()) {
            const Move& last = sequence.back();
            int last_id = last.axis * 3 + last.layer;
            if (current_id == last_id) {
                continue;
            }
        }
        if (sequence.size() >= 2) {
            const Move& second_last = sequence[sequence.size() - 2];
            int second_last_id = second_last.axis * 3 + second_last.layer;
            if (current_id == second_last_id) {
                continue;
            }
        }
        Move m;
        m.axis = axis;
        m.layer = layer;
        m.turns = turns_dist(gen);
        if (m.turns == 2) {
            m.dir = 1;
        } else {
            m.dir = (dir_dist(gen) == 0) ? 1 : -1;
        }
        sequence.push_back(m);
    }
    return sequence; 
}
}  // namespace cube
