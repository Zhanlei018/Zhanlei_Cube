# shaders —— GLSL 着色器

阶段 2 建两个文件（文件名随意，但建议）：

```
cube.vert
cube.frag
```

## 顶点数据约定（和 `src/render/mesh.hpp` 对齐）

| location | 类型 | 含义 |
| --- | --- | --- |
| 0 | `vec3` | 位置，局部坐标范围 [-0.5, 0.5] |
| 1 | `vec3` | 法线（每面独立，不能共用顶点） |
| 2 | `vec3` | 这一面的颜色（**初始**颜色，之后由逻辑状态提供） |

## 矩阵约定

三个 uniform（名字自己定，保持一致就行）：

```
uniform mat4 uModel;   // 单个 cubie 的模型矩阵，由 CPU 端拼好（含动画旋转）
uniform mat4 uView;
uniform mat4 uProj;
```

顶点着色器里就是 `uProj * uView * uModel * vec4(aPos, 1.0)`。
**顺序不要动**，从右往左作用在顶点上。

## 光照建议

别上 PBR。一个固定方向的平行光 + 环境光就够，公式简单到可以手推：

```
float ndl     = max(dot(N, L), 0.0);
vec3  lit     = baseColor * (0.35 + 0.65 * ndl);
```

再给每个面叠一点点法线方向的亮度差（比如按 `abs(N.y)` 微调），
方块之间的边界会清楚很多，视觉上立刻"高级"起来。

## 热重载（强烈建议）

Debug 下加个 `R` 键：重新读文件 → 编译 → 成功就换程序对象，失败就打印 infoLog
并**保留旧程序**（不要一编译失败画面就全黑）。

`shader.cpp` 里编译失败时务必把 `glGetShaderInfoLog` 打全，
包括行号 —— 没有行号的 GLSL 报错基本没法看。

## 注意

- 文件保存成 **LF 换行**（`.gitattributes` 已经管了）
- GLSL 版本声明用 `#version 330 core`，和 CMake 里的 `GLAD_API gl=3.3` 对齐
- 有些驱动对文件末尾的空白和 BOM 敏感，出怪问题时先看这里
