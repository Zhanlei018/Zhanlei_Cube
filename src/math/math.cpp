#include "math/math.hpp"
namespace cube {

int layerOf(int coord) {
    return coord + 1;
};

int axisIndex(char axis) {
    switch (axis) {
        case 'X':
        case 'x':
            return 0;
            break;
        case 'Y':
        case 'y':
            return 1;
            break;
        case 'Z':
        case 'z':
            return 2;
            break;
        default:
            return -10;  // 参数不合法，返回错误码-10,代表轴错误
            break;
    }
};

glm::imat3 rotX90() {
    glm::imat3 Matrix(0);
    Matrix[0][0] = 1;
    Matrix[1][2] = 1;
    Matrix[2][1] = -1;
    return Matrix;
}

glm::imat3 rotX(int turns) {
    const glm::imat3 X90 = rotX90();
    glm::imat3 res(1);
    for (int i = 0; i < turns; i++) {
        res = X90 * res;
    }
    return res;
}

glm::imat3 rotY90() {
    glm::imat3 Matrix(0);
    Matrix[0][2] = -1;
    Matrix[1][1] = 1;
    Matrix[2][0] = 1;
    return Matrix;
}

glm::imat3 rotY(int turns) {
    const glm::imat3 Y90 = rotY90();
    glm::imat3 res(1);
    for (int i = 0; i < turns; i++) {
        res = Y90 * res;
    }
    return res;
}
glm::imat3 rotZ90() {
    glm::imat3 Matrix(0);
    Matrix[0][1] = 1;
    Matrix[2][2] = 1;
    Matrix[1][0] = -1;
    return Matrix;
}

glm::imat3 rotZ(int turns) {
    const glm::imat3 Z90 = rotZ90();
    glm::imat3 res(1);
    for (int i = 0; i < turns; i++) {
        res = Z90 * res;
    }
    return res;
};
}  // namespace cube
