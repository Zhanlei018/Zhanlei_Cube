#include "state.hpp"

#include <iostream>

namespace cube {
Cubie makeSolvedCubie(glm::ivec3 pos) {
    return Cubie{pos, glm::imat3(1)};
}

State makeSolvedState() {
    State state_cubie;
    int index = 0;
    for (int i = -1; i <= 1; i++) {
        for (int j = -1; j <= 1; j++) {
            for (int k = -1; k <= 1; k++) {
                if (i == 0 && j == 0 && k == 0)
                    continue;
                Cubie block = makeSolvedCubie(glm::ivec3(i, j, k));
                state_cubie.cubies[index] = block;
                index++;
            }
        }
    }
    return state_cubie;
}

Color colorOfSticker(int columnIndex, bool reversed) {
    switch (columnIndex) {
        case 0: return reversed ? Color::kOrange : Color::kRed;
        case 1: return reversed ? Color::kYellow : Color::kWhite;
        case 2: return reversed ? Color::kBlue   : Color::kGreen;
        default: return Color::kUnknown;
    }
}

bool isSolved(const State& state) {
    const char faceOri[] = {'U', 'D', 'L', 'R', 'F', 'B'};
    for (size_t i = 0; i < 6; i++) {
        const std::array<Color, 9> face = faceColors(state, faceOri[i]);
        for (int j = 1; j < 8; j++) {   // 比较 8 对相邻元素：0-1, 1-2, ... 7-8
            if (face[j - 1] != face[j])
                return false;
        }
    }
    return true;
}

std::array<Color, 9> faceColors(const State& state, char face) {
    std::array<Color, 9> fc_cubie;
    glm::ivec3 faceDir;  // 定义面法线
    switch (face) {      // face参数转面法线向量
        case 'U':
            faceDir = glm::ivec3(0, 1, 0);
            break;
        case 'D':
            faceDir = glm::ivec3(0, -1, 0);
            break;
        case 'L':
            faceDir = glm::ivec3(-1, 0, 0);
            break;
        case 'R':
            faceDir = glm::ivec3(1, 0, 0);
            break;
        case 'F':
            faceDir = glm::ivec3(0, 0, 1);
            break;
        case 'B':
            faceDir = glm::ivec3(0, 0, -1);
            break;
        default:
            return {};
    }
    for (size_t i = 0; i < state.cubies.size(); i++) {  // 最外层遍历26个小方块
        glm::ivec3 pos = state.cubies[i].pos;
        bool onFace = false;
        for (int a = 0; a < 3; ++a) {  // 用两个if去判断方块是否在对应面上
            if (faceDir[a] != 0 && pos[a] == faceDir[a]) {
                onFace = true;
                break;
            }
        }
        if (!onFace)
            continue;
        int row = 0, col = 0;
        if (faceDir.x == 1) {  // 判断方块面朝向
            row = 1 - pos.y;
            col = 1 - pos.z;
        } else if (faceDir.x == -1) {
            row = 1 - pos.y;
            col = pos.z + 1;
        } else if (faceDir.y == 1) {
            row = pos.z + 1;
            col = pos.x + 1;
        } else if (faceDir.y == -1) {
            row = 1 - pos.z;
            col = pos.x + 1;
        } else if (faceDir.z == 1) {
            row = 1 - pos.y;
            col = pos.x + 1;
        } else if (faceDir.z == -1) {
            row = 1 - pos.y;
            col = 1 - pos.x;
        }
        int index = row * 3 + col;  // 用index整理方块的视觉3*3顺序，输入数组
        for (int j = 0; j < 3; j++) {
            const glm::ivec3 axis = state.cubies[i].orient[j];
            if (axis == faceDir) {
                fc_cubie[index] = colorOfSticker(j, false);
                break;
            }
            if (axis == -faceDir) {
                fc_cubie[index] = colorOfSticker(j, true);
                break;
            }
        }
    }
    return fc_cubie;
}

bool checkInvariants(const State& state) {
    for (int i = 0; i < 26; i++) {
        glm::ivec3 vec = state.cubies[i].pos;
        for (int j = 0; j <= 2; j++) {
            if (vec[j] > 1 || vec[j] < -1)
                return false;
        }

        glm::imat3 m = state.cubies[i].orient;
        if (m[0][0] * (m[1][1] * m[2][2] - m[2][1] * m[1][2]) -
                m[1][0] * (m[0][1] * m[2][2] - m[2][1] * m[0][2]) +
                m[2][0] * (m[0][1] * m[1][2] - m[1][1] * m[0][2]) !=
            1)
            return false;
        if (i > 0) {
            if (vec == state.cubies[i - 1].pos)
                return false;
        }
    }

    return true;
}

void printState(const State& state) {
    const char faceOri[] = {'U', 'D', 'L', 'R', 'F', 'B'};
    std::array<std::array<Color, 9>, 6> allFaceColors;
    for (int i = 0; i < 6; ++i) {
        allFaceColors[i] = faceColors(state, faceOri[i]);
    }
    std::array<std::array<char, 9>, 6> charGrid;
    for (int i = 0; i < 6; i++) {
        for (int j = 0; j < 9; j++) {
            charGrid[i][j] = colorChar(allFaceColors[i][j]);
        }
    }
    std::cout << "\n========魔方状态========\n\n";
    for (int row = 0; row < 3; ++row) {  // U面
        std::cout << "      ";
        for (int col = 0; col < 3; ++col) {
            std::cout << charGrid[0][row * 3 + col] << " ";
        }
        std::cout << "\n";
    }
    for (int row = 0; row < 3; ++row) {  // L面
        for (int col = 0; col < 3; ++col) std::cout << charGrid[2][row * 3 + col] << " ";
        std::cout << "  ";
        for (int col = 0; col < 3; ++col)  // F面
            std::cout << charGrid[4][row * 3 + col] << " ";
        std::cout << "  ";
        for (int col = 0; col < 3; ++col)  // R面
            std::cout << charGrid[3][row * 3 + col] << " ";
        std::cout << "  ";
        for (int col = 0; col < 3; ++col)  // B面
            std::cout << charGrid[5][row * 3 + col] << " ";

        std::cout << "\n";
    }
    for (int row = 0; row < 3; ++row) {  // D面
        std::cout << "      ";
        for (int col = 0; col < 3; ++col) {
            std::cout << charGrid[1][row * 3 + col] << " ";
        }
        std::cout << "\n";
    }
    std::cout << std::endl;
}

bool operator==(const State& a, const State& b) {
    if (a.cubies.size() != b.cubies.size())
        return false;
    for (size_t i = 0; i < a.cubies.size(); i++) {
        if (a.cubies[i].orient != b.cubies[i].orient)
            return false;
        if (a.cubies[i].pos != b.cubies[i].pos)
            return false;
    }
    return true;
}
}  // namespace cube