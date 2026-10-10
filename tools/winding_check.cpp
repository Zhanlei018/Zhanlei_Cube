// tools/winding_check.cpp —— 验证 mesh.cpp 的三角形绕序，并给出修正建议
//
// 做两件事：
//   1. 调用真正的 createCubieMesh()，把 GPU 缓冲读回来，算每个三角形的法线方向
//   2. 把顶点数据按"面"分组，用面自身的 u/v 向量独立算一次 2D 定向，
//      打印出每个面应该用哪种索引顺序 —— 这样你可以直接照抄结果
//
// 需要 GL 上下文，所以开一个隐藏窗口。

#include "render/gl_headers.hpp"
#include "render/mesh.hpp"

#include <cmath>
#include <cstdio>
#include <vector>

#if defined(_WIN32)
#include <windows.h>
#endif

int main() {
#if defined(_WIN32)
    SetConsoleOutputCP(CP_UTF8);
#endif

    if (glfwInit() != GLFW_TRUE) {
        std::printf("glfwInit 失败\n");
        return 1;
    }
    glfwWindowHint(GLFW_VISIBLE, GLFW_FALSE);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

    GLFWwindow* win = glfwCreateWindow(64, 64, "probe", nullptr, nullptr);
    if (!win) {
        std::printf("创建窗口失败\n");
        glfwTerminate();
        return 1;
    }
    glfwMakeContextCurrent(win);
    if (gladLoadGL(glfwGetProcAddress) == 0) {
        std::printf("gladLoadGL 失败\n");
        glfwTerminate();
        return 1;
    }

    cube::CubieMesh mesh = cube::createCubieMesh();

    // ---------- 读回缓冲 ----------
    glBindVertexArray(mesh.vao);

    GLint vboSize = 0;
    glBindBuffer(GL_ARRAY_BUFFER, mesh.vbo);
    glGetBufferParameteriv(GL_ARRAY_BUFFER, GL_BUFFER_SIZE, &vboSize);
    std::vector<float> vdata(static_cast<size_t>(vboSize) / sizeof(float));
    glGetBufferSubData(GL_ARRAY_BUFFER, 0, vboSize, vdata.data());

    GLint eboSize = 0;
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, mesh.ebo);
    glGetBufferParameteriv(GL_ELEMENT_ARRAY_BUFFER, GL_BUFFER_SIZE, &eboSize);
    std::vector<GLuint> idx(static_cast<size_t>(eboSize) / sizeof(GLuint));
    glGetBufferSubData(GL_ELEMENT_ARRAY_BUFFER, 0, eboSize, idx.data());

    glBindVertexArray(0);

    const int stride = 6;
    const int vcount = vboSize / (stride * static_cast<int>(sizeof(float)));
    std::printf("顶点数 = %d，索引数 = %zu\n\n", vcount, idx.size());

    if (vcount < 24 || idx.size() < 36) {
        std::printf("!! 缓冲数据不完整\n");
        glfwDestroyWindow(win);
        glfwTerminate();
        return 1;
    }

    const char* names[6] = {"+X", "-X", "+Y", "-Y", "+Z", "-Z"};
    const float expect[6][3] = {
        {1,0,0}, {-1,0,0}, {0,1,0}, {0,-1,0}, {0,0,1}, {0,0,-1}
    };

    std::printf("===== 第一份：从 GPU 缓冲读回来的实际绕序 =====\n");
    std::printf("%-4s %-22s %-22s %s\n", "面", "三角形1 法线", "三角形2 法线", "判定");
    std::printf("------------------------------------------------------------------------\n");

    int bad = 0;
    for (int f = 0; f < 6; ++f) {
        const GLuint base = static_cast<GLuint>(f * 3 * 2);
        float triN[2][3] = {{0,0,0},{0,0,0}};
        for (int t = 0; t < 2; ++t) {
            const GLuint i0 = idx[base + static_cast<GLuint>(t*3 + 0)];
            const GLuint i1 = idx[base + static_cast<GLuint>(t*3 + 1)];
            const GLuint i2 = idx[base + static_cast<GLuint>(t*3 + 2)];
            const float* p0 = &vdata[i0 * stride];
            const float* p1 = &vdata[i1 * stride];
            const float* p2 = &vdata[i2 * stride];
            const float e1[3] = {p1[0]-p0[0], p1[1]-p0[1], p1[2]-p0[2]};
            const float e2[3] = {p2[0]-p0[0], p2[1]-p0[1], p2[2]-p0[2]};
            triN[t][0] = e1[1]*e2[2] - e1[2]*e2[1];
            triN[t][1] = e1[2]*e2[0] - e1[0]*e2[2];
            triN[t][2] = e1[0]*e2[1] - e1[1]*e2[0];
        }
        auto fmt = [](const float* v) {
            static char buf[2][32];
            static int k = 0;
            char* b = buf[k++ % 2];
            std::snprintf(b, 32, "(%+.0f,%+.0f,%+.0f)", v[0], v[1], v[2]);
            return b;
        };
        const float d1 = triN[0][0]*expect[f][0] + triN[0][1]*expect[f][1] + triN[0][2]*expect[f][2];
        const float d2 = triN[1][0]*expect[f][0] + triN[1][1]*expect[f][1] + triN[1][2]*expect[f][2];
        const bool ok = (d1 > 0 && d2 > 0);
        if (!ok) ++bad;
        std::printf("%-4s %-22s %-22s %s\n", names[f],
                    fmt(triN[0]), fmt(triN[1]),
                    ok ? "朝外 OK" : ">>> 朝内 <<<");
    }

    // ---------- 第二份：独立推算每个面该用哪种顺序 ----------
    //
    // 不看索引缓冲，只看顶点数据本身。
    // 面法线 n = normalize(p3 - p0) × normalize(p1 - p0)
    //   （取两条对角线叉乘，避免受索引顺序影响）
    // 然后把四个角投影到 (u = p1-p0, v = p3-p0) 平面上，算三角形的 2D 有向面积。
    // 面积为正 = 从 +n 方向看是逆时针 = 正面。
    std::printf("\n===== 第二份：每个面独立推算，并给出修正建议 =====\n");
    std::printf("%-4s %-24s %-24s %s\n", "面", "实际面法线", "应有法线", "结论");
    std::printf("------------------------------------------------------------------------\n");

    for (int f = 0; f < 6; ++f) {
        const float* p0 = &vdata[static_cast<size_t>(f*4 + 0) * stride];
        const float* p1 = &vdata[static_cast<size_t>(f*4 + 1) * stride];
        const float* p2 = &vdata[static_cast<size_t>(f*4 + 2) * stride];
        const float* p3 = &vdata[static_cast<size_t>(f*4 + 3) * stride];

        // 两条对角线叉乘 -> 面法线（与绕序无关）
        const float d1[3] = {p2[0]-p0[0], p2[1]-p0[1], p2[2]-p0[2]};
        const float d2[3] = {p3[0]-p1[0], p3[1]-p1[1], p3[2]-p1[2]};
        float n[3] = {d1[1]*d2[2]-d1[2]*d2[1], d1[2]*d2[0]-d1[0]*d2[2], d1[0]*d2[1]-d1[1]*d2[0]};
        const float nl = std::sqrt(n[0]*n[0]+n[1]*n[1]+n[2]*n[2]);
        if (nl > 1e-9f) { n[0]/=nl; n[1]/=nl; n[2]/=nl; }

        // 用 p1-p0 和 p3-p0 做屏幕基
        const float u[3] = {p1[0]-p0[0], p1[1]-p0[1], p1[2]-p0[2]};
        const float v[3] = {p3[0]-p0[0], p3[1]-p0[1], p3[2]-p0[2]};

        // 三角形的 2D 有向面积（在当前索引顺序 (0,1,2) 下）
        // 投影到 (u,v) 基上的坐标：角 0 = (0,0)，角 1 = (1,0) 比例，
        // 直接算叉乘在 n 方向的投影即可
        const float e1[3] = {p1[0]-p0[0], p1[1]-p0[1], p1[2]-p0[2]};
        const float e2[3] = {p2[0]-p0[0], p2[1]-p0[1], p2[2]-p0[2]};
        const float cr[3] = {e1[1]*e2[2]-e1[2]*e2[1], e1[2]*e2[0]-e1[0]*e2[2], e1[0]*e2[1]-e1[1]*e2[0]};
        const float dot = cr[0]*n[0] + cr[1]*n[1] + cr[2]*n[2];

        // n 应该朝外（等于 expect[f]）
        const float outDot = n[0]*expect[f][0] + n[1]*expect[f][1] + n[2]*expect[f][2];
        const bool normalOutward = outDot > 0;
        const bool currentCCW = dot > 0;   // 当前 (0,1,2) 顺序是不是正面

        // 要让它成为正面：(法线朝外) 且 (从外看逆时针)
        // 当前索引顺序若是逆时针且法线朝外 -> 已经对了
        const bool alreadyOK = normalOutward && currentCCW;

        auto fmt3 = [](const float* w, char* b) {
            std::snprintf(b, 32, "(%+.0f,%+.0f,%+.0f)", w[0], w[1], w[2]);
        };
        char bn[32], be[32];
        fmt3(n, bn); fmt3(expect[f], be);

        std::printf("%-4s %-24s %-24s %s\n", names[f], bn, be,
                    alreadyOK ? "正序(b+0,b+1,b+2) 已正确"
                              : ">>> 需要用【反序】(b+0,b+2,b+1) <<<");
    }

    std::printf("\n");
    if (bad == 0) {
        std::printf("第一份检查：六个面绕序全部正确。\n");
    } else {
        std::printf("第一份检查：%d 个面绕序反了 —— 照第二份的结论改。\n", bad);
    }

    cube::destroyMesh(mesh);
    glfwDestroyWindow(win);
    glfwTerminate();
    return 0;
}
