#include "cube/moves.hpp"
#include "cube/state.hpp"
#include "render/camera.hpp"
#include "render/color.hpp"
#include "render/gl_headers.hpp"
#include "render/mesh.hpp"
#include "render/shader.hpp"
// GLM
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/quaternion.hpp>
#include <glm/mat4x4.hpp>
#include <glm/vec3.hpp>
// 标准库
#include <algorithm>
#include <cstdio>
#include <deque>
#include <random>
#include <string>
// Windows
#if defined(_WIN32)
#include <windows.h>
#endif

cube::OrbitCamera camera;
bool g_dragging = false;
double g_lastX = 0.0;
double g_lastY = 0.0;

void mouseButtonCallback(GLFWwindow* window, int button, int action, int mods) {
    if (button != GLFW_MOUSE_BUTTON_LEFT)
        return;
    if (action == GLFW_PRESS) {
        g_dragging = true;
        glfwGetCursorPos(window, &g_lastX, &g_lastY);
    } else if (action == GLFW_RELEASE) {
        g_dragging = false;
    }
}

void cursorPosCallback(GLFWwindow* window, double x, double y) {
    const float dx = static_cast<float>(x - g_lastX);
    const float dy = static_cast<float>(y - g_lastY);
    g_lastX = x;
    g_lastY = y;
    if (g_dragging)
        camera.orbit(dx, dy);
}

void scrollCallback(GLFWwindow* window, double xoff, double yoff) {
    camera.zoom(static_cast<float>(yoff));
    ;
}

void framebufferSizeCallback(GLFWwindow* window, int width, int height) {
    (void)window;
    glViewport(0, 0, width, height);
    camera.setViewport(width, height);
}

int main() {
#if defined(_WIN32)
    SetConsoleOutputCP(CP_UTF8);
#endif
    if (glfwInit() != GLFW_TRUE) {
        std::printf("【致命错误】 glfwInit初始化失败\n");
        return 1;
    }
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
    GLFWwindow* window = glfwCreateWindow(1280, 720, "GL_Cube", nullptr, nullptr);
    if (window == nullptr) {
        std::printf("【致命错误】 创建窗口失败\n");
        glfwTerminate();
        return 1;
    }
    glfwMakeContextCurrent(window);
    glfwSwapInterval(1);
    if (gladLoadGL(glfwGetProcAddress) == 0) {
        std::printf("【致命错误】 Glad上下文加载失败\n");
        glfwDestroyWindow(window);
        glfwTerminate();
        return 1;
    }
    std::printf("[GL] 渲染器: %s\n", glGetString(GL_RENDERER));
    std::printf("[GL] 版本  : %s\n", glGetString(GL_VERSION));
    glEnable(GL_DEPTH_TEST);
    cube::Shader shader;
    shader.loadFromFiles("shaders/cube.vert", "shaders/cube.frag");
    cube::CubieMesh mesh = cube::createCubieMesh();
    cube::State state = cube::makeSolvedState();
    // 启动时同步一次 viewport 和宽高比（窗口未必是 1280x720）
    {
        int fbW = 0;
        int fbH = 0;
        glfwGetFramebufferSize(window, &fbW, &fbH);
        glViewport(0, 0, fbW, fbH);
        camera.setViewport(fbW, fbH);
    }

    glfwSetFramebufferSizeCallback(window, framebufferSizeCallback);
    glfwSetMouseButtonCallback(window, mouseButtonCallback);
    glfwSetCursorPosCallback(window, cursorPosCallback);
    glfwSetScrollCallback(window, scrollCallback);
    while (glfwWindowShouldClose(window) == 0) {
        glfwPollEvents();
        glClearColor(0.10f, 0.11f, 0.14f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        shader.use();
        shader.setMat4("uView", &camera.viewMatrix()[0][0]);
        shader.setMat4("uProj", &camera.projectionMatrix()[0][0]);
        glBindVertexArray(mesh.vao);

        for (int i = 0; i < cube::kCubieCount; i++) {
            glm::mat4 model = glm::translate(glm::mat4(1.0f), glm::vec3(state.cubies[i].pos));
            model = glm::scale(model, glm::vec3(1.0f));
            shader.setMat4("uModel", &model[0][0]);
            for (int f = 0; f < 6; ++f) {
                glm::ivec3 dir(0);
                dir[f / 2] = (f % 2 == 0) ? 1 : -1;
                const glm::vec3 rgb = cube::colorRgb(cube::stickerColor(state.cubies[i], dir));
                shader.setVec3("uColor", &rgb[0]);
                glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT,
                               reinterpret_cast<void*>(static_cast<uintptr_t>(
                                   mesh.faceIndexOffset[f] * sizeof(GLuint))));
            }
        }
        glfwSwapBuffers(window);
    }
    cube::destroyMesh(mesh);
    shader.destroy();
    glfwDestroyWindow(window);
    glfwTerminate();
    return 0;
}