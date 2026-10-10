// src/render/shader.hpp —— 着色器程序
//
// 【你要实现这个文件对应的 .cpp】
//
// ---------------------------------------------------------------------------
// 验收标准
// ---------------------------------------------------------------------------
//   1. 着色器文件正常时，程序能编译链接成功
//   2. 着色器写错时：
//        - 终端打印出【带行号】的源码，和驱动的错误信息
//        - 上一个能用的程序【仍然有效】，画面不会变黑
//        - 程序不崩溃
//
// 第 2 条是这个类存在的意义。写 GLSL 一定会写错，
// 如果一写错画面就全黑，你连自己改的是哪儿都看不见。
//
// ---------------------------------------------------------------------------
// 卡住时可以问自己的问题
// ---------------------------------------------------------------------------
//   - 着色器编译/链接失败，怎么知道失败原因？（不是靠猜，OpenGL 有接口）
//   - 顶点和片元两个阶段，哪个失败了？失败的 stage 对象要不要释放？
//   - 链接完成之后，那两个 stage 对象还需要保留吗？
//   - uniform 名字写错了会怎样？glGetUniformLocation 返回什么？
//     这时候要不要报错？
//   - 类持有 GL 资源（一个 GLuint），拷贝构造会发生什么？该不该禁止？
//
// ---------------------------------------------------------------------------
// 提示
// ---------------------------------------------------------------------------
//   - GLSL 要告诉驱动"用的是哪个版本"，在第一行写 #version 330 core
//   - 读文件用 std::ifstream
//   - 调试信息通过 glGetShaderiv / glGetShaderInfoLog 拿到
//   - GLM 的 glm/gtc/type_ptr.hpp 里有 &mat[0][0] 这种传矩阵指针的写法

#pragma once

#include <string>

#include "render/gl_headers.hpp"

namespace cube {
inline constexpr GLuint kAttrPos = 0;
inline constexpr GLuint kAttrNormal = 1;

class Shader {
public:
    Shader() = default;
    ~Shader();
    Shader(const Shader&) = delete;
    Shader& operator=(const Shader&) = delete;

    bool loadFromFiles(const std::string& vertPath, const std::string& fragPath);
    void use() const;
    void destroy();
    bool valid() const { return program_ != 0; }
    void setMat4(const char* name, const float* value) const;
    void setVec3(const char* name, const float* value) const;

private:
    GLuint program_ = 0;
    std::string vertPath_;
    std::string fragPath_;
};

}  // namespace cube
