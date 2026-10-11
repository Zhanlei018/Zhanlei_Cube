#pragma once

#include <cstdint>
#include <vector>

#include "cube/state.hpp"

namespace cube {
std::vector<Move> randomScramble(int steps = 25, std::uint32_t seed = 20261011u);
}  // namespace cube
