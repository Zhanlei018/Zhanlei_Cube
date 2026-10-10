#include "shader.hpp"

#include <fstream>
#include <iostream>
#include <sstream>
#include <vector>

namespace cube {
Shader::~Shader() {
    destroy();
}
bool Shader::loadFromFiles(const std::string& vertPath, const std::string& fragPath) {
    std::string vertexCode;
    std::string fragmentCode;
    std::ifstream vShaderFile;
    std::ifstream fShaderFile;
    vertPath_ = vertPath;
    fragPath_ = fragPath;
    vShaderFile.exceptions(std::ifstream::failbit | std::ifstream::badbit);
    fShaderFile.exceptions(std::ifstream::failbit | std::ifstream::badbit);
    try {
        vShaderFile.open(vertPath_);
        fShaderFile.open(fragPath_);
        std::stringstream vShaderStream, fShaderStream;
        vShaderStream << vShaderFile.rdbuf();
        fShaderStream << fShaderFile.rdbuf();
        vShaderFile.close();
        fShaderFile.close();
        vertexCode = vShaderStream.str();
        fragmentCode = fShaderStream.str();
    } catch (std::ifstream::failure& e) {
        std::cerr << "【错误】 shader 无法正确读取文件" << e.what() << std::endl;
        return false;
    }
    const char* vShaderCode = vertexCode.c_str();
    const char* fShaderCode = fragmentCode.c_str();
    GLuint vertex = glCreateShader(GL_VERTEX_SHADER);
    glShaderSource(vertex, 1, &vShaderCode, nullptr);
    glCompileShader(vertex);
    GLint vSuccess;
    glGetShaderiv(vertex, GL_COMPILE_STATUS, &vSuccess);
    if (!vSuccess) {
        GLint len = 0;
        glGetShaderiv(vertex, GL_INFO_LOG_LENGTH, &len);
        std::vector<char> log(static_cast<size_t>(len > 1 ? len : 1), '\0');
        glGetShaderInfoLog(vertex, len, nullptr, log.data());
        std::cerr << log.data() << std::endl;
        std::cerr << "【错误】顶点着色器编译失败:\n";
        std::istringstream iss(vertexCode);
        std::string line;
        int n = 1;
        while (std::getline(iss, line)) {
            std::printf("%3d | %s\n", n, line.c_str());
            ++n;
        }
        glDeleteShader(vertex);
        return false;
    }
    GLuint fragment = glCreateShader(GL_FRAGMENT_SHADER);
    glShaderSource(fragment, 1, &fShaderCode, nullptr);
    glCompileShader(fragment);
    GLint fSuccess;
    glGetShaderiv(fragment, GL_COMPILE_STATUS, &fSuccess);
    if (!fSuccess) {
        GLint len = 0;
        glGetShaderiv(fragment, GL_INFO_LOG_LENGTH, &len);
        std::vector<char> log(static_cast<size_t>(len > 1 ? len : 1), '\0');  
        glGetShaderInfoLog(fragment, len, nullptr, log.data());
        std::cerr << log.data() << std::endl;
        std::cerr << "【错误】片元着色器编译失败:\n";
        std::istringstream iss(fragmentCode);
        std::string line;
        int n = 1;
        while (std::getline(iss, line)) {
            std::printf("%3d | %s\n", n, line.c_str());
            ++n;
        }
        glDeleteShader(fragment);
        glDeleteShader(vertex);
        return false;
    }
    const GLuint prog = glCreateProgram();
    glAttachShader(prog, vertex);
    glAttachShader(prog, fragment);
    glLinkProgram(prog);
    GLint success;
    glGetProgramiv(prog, GL_LINK_STATUS, &success);
    if (!success) {
        glDeleteProgram(prog);
        std::cerr << "【错误】 shader 链接文件失败" << std::endl;
        return false;
    }
    destroy();
    program_ = prog;
    glDeleteShader(vertex);
    glDeleteShader(fragment);
    return true;
}

void Shader::use() const {
    glUseProgram(program_);
}

void Shader::destroy() {
    if (program_ != 0) {
        glDeleteProgram(program_);
        program_ = 0;
    }
}

void Shader::setMat4(const char* name, const float* value) const {
    if (!valid()) {
        return;
    }
    glUseProgram(program_);
    GLint location = glGetUniformLocation(program_, name);
    if (location != -1) {
        glUniformMatrix4fv(location, 1, GL_FALSE, value);
    }
}

void Shader::setVec3(const char* name, const float* value) const {
    if (!valid()) {
        return;
    }
    glUseProgram(program_);
    GLint location = glGetUniformLocation(program_, name);
    if (location != -1) {
        glUniform3fv(location, 1, value);
    }
}
}  // namespace cube