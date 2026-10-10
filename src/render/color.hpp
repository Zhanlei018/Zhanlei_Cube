#pragma once

#include <glm/vec3.hpp>

#include "cube/state.hpp"

namespace cube {
inline glm::vec3 colorRgb(Color c) {
    switch (c) {
        case Color::kRed:
            return {0.85f, 0.16f, 0.16f};  // +X
        case Color::kOrange:
            return {0.95f, 0.52f, 0.10f};  // -X
        case Color::kWhite:
            return {0.93f, 0.93f, 0.90f};  // +Y
        case Color::kYellow:
            return {0.95f, 0.85f, 0.20f};  // -Y
        case Color::kGreen:
            return {0.10f, 0.62f, 0.28f};  // +Z
        case Color::kBlue:
            return {0.12f, 0.36f, 0.80f};  // -Z
        default:
            return {1.00f, 0.00f, 1.00f};  // 醒目的错误颜色
    }
}

inline Color stickerColor(const Cubie& cubie, const glm::ivec3& worldDir) {
    for (int c = 0; c < 3; ++c) {
        const glm::ivec3 axis = cubie.orient[c];
        if (axis == worldDir) {
            return colorOfSticker(c, false);
        }
        if (axis == -worldDir) {
            return colorOfSticker(c, true);
        }
    }
    return Color::kUnknown;
}

}  // namespace cube
