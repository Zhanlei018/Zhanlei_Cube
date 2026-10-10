#pragma once

#include <array>
#include <string>
#include <vector>

#include "cube/cubie.hpp"

namespace cube {
enum class Color : int {
    kUnknown = 0,  // 出错
    kRed,  // +X
    kOrange,   // -X
    kWhite,    // +Y
    kYellow,   // -Y
    kGreen,    // +Z
    kBlue,     // -Z
};
inline char colorChar(Color c) {
    switch (c) {
        case Color::kRed:
            return 'R';
        case Color::kOrange:
            return 'O';
        case Color::kWhite:
            return 'W';
        case Color::kYellow:
            return 'Y';
        case Color::kGreen:
            return 'G';
        case Color::kBlue:
            return 'B';
        default:
            return '?';
    }
}
enum Axis : int {
    kAxisX = 0,
    kAxisY = 1,
    kAxisZ = 2,
};
struct Move {
    int axis = kAxisX;
    int layer = 0;
    int dir = 1;
    int turns = 1;
};
inline constexpr int kCubieCount = 26;
struct State {
    std::array<Cubie, kCubieCount> cubies;
    std::vector<Move> history;
};
State makeSolvedState();
bool isSolved(const State& state);
std::array<Color, 9> faceColors(const State& state, char face);
void printState(const State& state);
Color colorOfSticker(const glm::ivec3& column);
bool checkInvariants(const State& state);
bool operator==(const State& a, const State& b);
inline bool operator!=(const State& a, const State& b) {
    return !(a == b);
}
}  // namespace cube
