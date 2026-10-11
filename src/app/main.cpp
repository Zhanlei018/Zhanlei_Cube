#include "anim/animator.hpp"
#include "cube/moves.hpp"
#include "cube/state.hpp"
#include "solver/solver.hpp"
#include "input/drag.hpp"
#include "input/picking.hpp"
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
cube::State state;
cube::DragAnchor g_anchor;
cube::DragState g_drag;      // 拖动预览状态
bool g_dragging = false;
double g_lastX = 0.0;
double g_lastY = 0.0;
double g_mouseX = 0.0;
double g_mouseY = 0.0;
int g_fbW = 1280;
int g_fbH = 720;
bool g_orbiting = false;
bool g_restorePending = false;
cube::Animator g_animator;

cube::Ray rayFromMouse() {
    return cube::makeRayFromScreen(static_cast<int>(g_mouseX), static_cast<int>(g_mouseY), g_fbW,
                                   g_fbH, camera.viewMatrix(), camera.projectionMatrix());
}

void mouseButtonCallback(GLFWwindow* window, int button, int action, int mods) {
    g_orbiting = false;
    if (button != GLFW_MOUSE_BUTTON_LEFT)
        return;
    if (action == GLFW_PRESS) {
        const cube::PickResult pick =
            cube::pickAtScreen(static_cast<int>(g_mouseX), static_cast<int>(g_mouseY), g_fbW, g_fbH,
                               camera.viewMatrix(), camera.projectionMatrix());

        g_anchor = cube::makeAnchor(pick);
        cube::beginDrag(g_drag, g_anchor);

        if (g_anchor.valid) {
            g_dragging = true;
            std::printf("【锚点】cell=(%d,%d,%d) normal=(%d,%d,%d)\n", pick.cell.x, pick.cell.y,
                        pick.cell.z, pick.normal.x, pick.normal.y, pick.normal.z);
        } else {
            g_orbiting = true;
        }
        glfwGetCursorPos(window, &g_lastX, &g_lastY);
    } else if (action == GLFW_RELEASE) {
        if (g_dragging && g_drag.valid && g_drag.active) {
            const float kCommitProgress = 0.6f;
            const float progress = std::abs(g_drag.angle) / glm::half_pi<float>();

            if (progress >= kCommitProgress) {
                cube::Move m;
                m.axis = g_drag.axis;
                m.layer = g_drag.layer;
                m.dir = g_drag.dir;
                m.turns = 1;
                g_animator.beginMove(m, progress);
                std::printf("【拖动】axis=%d layer=%d dir=%+d  已转 %.0f%%，播完剩余\n", m.axis,
                            m.layer, m.dir, progress * 100.0f);
            } else {
                std::printf("【拖动】只转了 %.0f%%，取消\n", progress * 100.0f);
            }
        }
        cube::endDrag(g_drag);
        g_dragging = false;
        g_anchor.valid = false;
    }
}

void cursorPosCallback(GLFWwindow* window, double x, double y) {
    const float dx = static_cast<float>(x - g_lastX);
    const float dy = static_cast<float>(y - g_lastY);
    g_lastX = x;
    g_lastY = y;
    g_mouseX = x;
    g_mouseY = y;
    if (g_orbiting)
        camera.orbit(dx, dy);
}

void scrollCallback(GLFWwindow* window, double xoff, double yoff) {
    camera.zoom(static_cast<float>(yoff));
}

void framebufferSizeCallback(GLFWwindow* window, int width, int height) {
    (void)window;
    glViewport(0, 0, width, height);
    camera.setViewport(width, height);
    g_fbW = width;
    g_fbH = height;
}

void keyCallback(GLFWwindow* window, int key, int scancode, int action, int mods) {
    if (action != GLFW_PRESS)
        return;
    const bool ctrl = (mods & GLFW_MOD_CONTROL) != 0;
    if (ctrl && key == GLFW_KEY_R) {
        const std::vector<cube::Move> undo = cube::invertedSequence(state.history);
        g_animator.pushSequence(undo);
        g_restorePending = true;
        std::printf("【复原】排入 %zu 个动作\n", undo.size());
        return;
    }
    if (key == GLFW_KEY_S) {
        g_animator.clearHistory(state);
        const std::vector<cube::Move> scramble = cube::randomScramble(25);
        for (const cube::Move& m : scramble) {
            g_animator.applySilent(state, m);
            g_animator.push(m);
        }
        std::printf("【打乱】%zu 步\n", scramble.size());
        return;
    }

    if (key == GLFW_KEY_ESCAPE) {
        glfwSetWindowShouldClose(window, GLFW_TRUE);
        return;
    }
    bool shift;
    shift = glfwGetKey(window, GLFW_KEY_LEFT_SHIFT) == GLFW_PRESS ||
            glfwGetKey(window, GLFW_KEY_RIGHT_SHIFT) == GLFW_PRESS;
    char face = '0';
    switch (key) {
        case GLFW_KEY_U:
            face = 'U';
            break;
        case GLFW_KEY_D:
            face = 'D';
            break;
        case GLFW_KEY_L:
            face = 'L';
            break;
        case GLFW_KEY_R:
            face = 'R';
            break;
        case GLFW_KEY_F:
            face = 'F';
            break;
        case GLFW_KEY_B:
            face = 'B';
            break;
        default:
            return;
    }
    std::string token(1, face);
    if (shift)
        token += "\'";
    std::optional<cube::Move> maybeMove = cube::parseMove(token);
    if (!maybeMove)
        return;
    cube::Move realmove = *maybeMove;
    g_animator.push(realmove);
    std::printf("【入队】 %s  队列里还有 %zu 个\n", token.c_str(), g_animator.pendingCount());
    (void)window;
    (void)key;
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
    state = cube::makeSolvedState();
    // 启动时同步一次 viewport 和宽高比（窗口未必是 1280x720）
    {
        int fbW = 0;
        int fbH = 0;
        glfwGetFramebufferSize(window, &fbW, &fbH);
        glViewport(0, 0, fbW, fbH);
        camera.setViewport(fbW, fbH);
        g_fbW = fbW;
        g_fbH = fbH;
    }
    glfwSetFramebufferSizeCallback(window, framebufferSizeCallback);
    glfwSetMouseButtonCallback(window, mouseButtonCallback);
    glfwSetCursorPosCallback(window, cursorPosCallback);
    glfwSetScrollCallback(window, scrollCallback);
    glfwSetKeyCallback(window, keyCallback);
    std::printf("【操作】:R/U/F/L/D/B 转面(按住 Shift 逆时针),Esc 退出\n\n");
    double lastTime = glfwGetTime();

    while (glfwWindowShouldClose(window) == 0) {
        glfwPollEvents();
        double dt = std::clamp(glfwGetTime() - lastTime, 0.0, 0.05);
        int fbW = 0, fbH = 0;
        glfwGetFramebufferSize(window, &fbW, &fbH);
        const cube::PickResult pick =
            cube::pickAtScreen(static_cast<int>(g_mouseX), static_cast<int>(g_mouseY), fbW, fbH,
                               camera.viewMatrix(), camera.projectionMatrix());
        lastTime = glfwGetTime();
        g_animator.update(state, dt);

        if (g_restorePending && !g_animator.isAnimating() && g_animator.pendingCount() == 0) {
            g_animator.clearHistory(state);
            g_restorePending = false;
            std::printf("【复原】完成，history 已清空\n");
        }

        // 拖动预览：用当前鼠标位置更新预览状态
        if (g_dragging && g_drag.valid) {
            cube::Ray mouseRay = cube::makeRayFromScreen(
                static_cast<int>(g_mouseX), static_cast<int>(g_mouseY), fbW, fbH,
                camera.viewMatrix(), camera.projectionMatrix());
            cube::updateDrag(g_drag, mouseRay);
        }

        const glm::mat4 animRot = g_animator.rotationMatrix();
        const glm::mat4 dragRot = cube::dragRotationMatrix(g_drag);
        glClearColor(0.10f, 0.11f, 0.14f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        shader.use();
        shader.setMat4("uView", &camera.viewMatrix()[0][0]);
        shader.setMat4("uProj", &camera.projectionMatrix()[0][0]);
        glBindVertexArray(mesh.vao);
        for (int i = 0; i < cube::kCubieCount; i++) {
            glm::mat4 model = glm::translate(glm::mat4(1.0f), glm::vec3(state.cubies[i].pos));
            model = glm::scale(model, glm::vec3(1.0f));
            const bool hovered = pick.hit && (state.cubies[i].pos == pick.cell);
            if (g_animator.isAnimating()) {
                cube::Move m = g_animator.currentMove();
                if (state.cubies[i].pos[m.axis] == m.layer - 1) {
                    model = animRot * model;
                }
            }
            // 拖动预览：该层的方块跟着鼠标转
            if (g_drag.active) {
                const glm::ivec3 dpos = state.cubies[i].pos;
                if (dpos[g_drag.axis] == g_drag.layer - 1) {
                    model = dragRot * model;
                }
            }
            shader.setMat4("uModel", &model[0][0]);
            for (int f = 0; f < 6; ++f) {
                glm::ivec3 dir(0);
                dir[f / 2] = (f % 2 == 0) ? 1 : -1;
                glm::vec3 rgb = cube::colorRgb(cube::stickerColor(state.cubies[i], dir));
                if (hovered) {
                    rgb = glm::min(rgb * 1.6f, glm::vec3(1.0f));
                }
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