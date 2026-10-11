#pragma once

#include <optional>
#include <string>
#include <vector>

#include "cube/state.hpp"

namespace cube {
using Layer = int;
void applyMove(State& state, const Move& move);
Move inverseOf(const Move& move);
std::optional<Move> parseMove(const std::string& token);
std::vector<Move> parseSequence(const std::string& text);
std::vector<Move> invertedSequence(const std::vector<Move>& moves);
}  // namespace cube
