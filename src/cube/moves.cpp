#include "moves.hpp"

#include <iostream>
#include <sstream>

#include "math/math.hpp"

namespace cube {
Move inverseOf(const Move& move) {
    Move move_inverse = move;
    move_inverse.dir = -move_inverse.dir;
    return move_inverse;
}

std::vector<Move> invertedSequence(const std::vector<Move>& moves) {
    std::vector<Move> move_inverted;
    for (int i = 0; i < moves.size(); i++) {
        move_inverted.push_back(inverseOf(moves[moves.size() - 1 - i]));
    }
    return move_inverted;
}

void applyMove(State& state, const Move& move) {
    int turns_move = move.turns % 4;
    if (move.dir == -1)
        turns_move = -turns_move + 4;
    glm::imat3 baseMat(1);
    glm::imat3 ultiMat(1);
    switch (move.axis) {
        case 0:
            baseMat = rotX90();
            break;
        case 1:
            baseMat = rotY90();
            break;
        case 2:
            baseMat = rotZ90();
            break;
        default:
            break;
    }
    for (int i = 0; i < turns_move; i++) {
        ultiMat = baseMat * ultiMat;
    }
    glm::ivec3 pos_temp;
    glm::imat3 orient_temp;
    for (int i = 0; i < kCubieCount; i++) {
        if (state.cubies[i].pos[move.axis] == move.layer - 1) {
            pos_temp = state.cubies[i].pos;
            orient_temp = state.cubies[i].orient;
            state.cubies[i].pos = ultiMat * pos_temp;
            state.cubies[i].orient = ultiMat * orient_temp;
        }
    }
}

std::optional<Move> parseMove(const std::string& token) {
    if (token.empty()) return std::nullopt;
    char face = std::toupper(token[0]);
    Move single_move{};
    switch (face) {
        case 'U':
            single_move.axis = kAxisY;
            single_move.layer = 2;
            break;
        case 'D':
            single_move.axis = kAxisY;
            single_move.layer = 0;
            break;
        case 'R':
            single_move.axis = kAxisX;
            single_move.layer = 2;
            break;
        case 'L':
            single_move.axis = kAxisX;
            single_move.layer = 0;
            break;
        case 'F':
            single_move.axis = kAxisZ;
            single_move.layer = 2;
            break;
        case 'B':
            single_move.axis = kAxisZ;
            single_move.layer = 0;
            break;
        default:
            std::cerr << "【错误】 无法识别的面:" << token;
            return std::nullopt;
    }
    if(token.length() == 1){
        single_move.turns = 1;
    }
    else if(token.length() == 2){
        if(token[1] == '\'') single_move.dir = -single_move.dir;
        else single_move.turns = token[1] - '0';
    }
    else if(token.length() == 3){
        if (token[1] != '\'') return std::nullopt;
        single_move.dir = -single_move.dir;
        single_move.turns = token[2];
    }
    else {
        std::cerr << "【错误】 不合法操作:" << token;
        return std::nullopt;
    }
    return single_move;
}

std::vector<Move> parseSequence(const std::string& text) {
    std::vector<Move> moves;
    std::istringstream iss(text);
    std::string token;
    while (iss >> token) {
        if (token.empty())
            continue;
        std::optional<Move> maybe = parseMove(token);
        if(!maybe.has_value()) {
            std::cerr << "【错误】 无法解析操作序列:" << token << ",停止解析并返回已解析内容";
            break;
        }
        Move single_move = *maybe;
        moves.push_back(single_move);
    }
    return moves;
}
}  // namespace cube