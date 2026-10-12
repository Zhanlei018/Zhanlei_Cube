# Cube —— 3x3x3 魔方（C++17 + OpenGL）

一个**边写边学**的项目：魔方逻辑、渲染管线、射线拾取、拖动交互、求解器全部自己接起来。

- **逻辑层**（`src/cube/`）纯数据结构，不碰任何图形 API —— 魔方转得对不对
  可以在终端里验证，不用开窗口
- **渲染层**（`src/render/` + `src/input/`）唯一碰 OpenGL 的地方
- **求解器**（`src/solver/`）接入 Kociemba 两阶段算法，按 `Q` 键即时求解

---

## 仓库结构

```
D:\Cube
├── CMakeLists.txt          构建脚本（含进度守卫，缺文件也能编）
├── CMakePresets.json       debug / release 两套预设
│
├── shaders/                cube.vert / cube.frag
│
├── src/
│   ├── math/         薄封装：整数旋转矩阵、格坐标 ↔ 层号
│   ├── cube/         ★ 逻辑模型：Cubie / State / Move / applyMove
│   ├── anim/         转动动画队列（纯时序逻辑，不碰 OpenGL）
│   ├── solver/       打乱 + 求解器
│   │   ├── solver.cpp            randomScramble + 旧的逐层法求解器
│   │   ├── twophase_solver.cpp   Kociemba 两阶段（封装 min2phase）
│   │   └── ...
│   ├── render/       程序化网格、着色器、轨道相机
│   ├── input/        射线拾取 + 拖动 → Move（含实时预览）
│   └── app/          main + 主循环 + 输入回调
│
├── tests/            纯逻辑单元测试（不链接任何图形库）
│
└── third_party/
    ├── khr/          离线依赖：khrplatform.h（GLAD 生成时需要）
    └── min2phase/    Kociemba 两阶段求解器（第三方，见该目录 README 的许可说明）
```

### 分层（构建系统强制）

**依赖方向只能向下，不能横向或向上。**

| 库 | 内容 | 能链接什么 |
| --- | --- | --- |
| `cube_core` | `math/` + `cube/` | 只有 GLM（整数类型） |
| `cube_anim` | `anim/` | cube_core + GLM |
| `cube_solver` | `solver/` | cube_core + GLM + min2phase |
| `cube_gfx` | `render/` + `input/` | 上面全部 + GLFW + GLAD |
| `cube_app` | `app/` | 以上全部 |

**这把分层写进了构建系统** —— `cube_anim` 和 `cube_solver` 不链接 GLFW/GLAD，
所以在里面写 `gl*` 调用会直接编译失败，比靠自觉可靠。

---

## 启动

```powershell
cd D:\Cube
cmake --preset debug                # 首次需要联网拉依赖，会慢一点
cmake --build --preset debug        # 构建全部
.\build\debug\cube_app.exe          # 运行
```

> ⚠️ **必须在 `D:\Cube` 目录下运行。**
> 着色器用的是相对路径 `shaders/cube.vert`，直接双击 `build\debug\cube_app.exe`
> 会因为工作目录不对而读不到着色器。

---

## 操作

### 鼠标

| 操作 | 效果 |
| --- | --- |
| **左键拖动魔方表面** | 对应层跟着转，松手后落位 |
| **左键拖动背景** | 绕魔方转视角 |
| **滚轮** | 推拉远近 |

拖动**不超过 60%** 就松手 → 取消，那一层弹回原位；
**超过 60%** → 预览锁定在 90°，松手后动画补完剩余部分。

### 键盘

| 按键 | 效果 |
| --- | --- |
| `R` `U` `F` `L` `D` `B` | 转对应面（右 / 上 / 前 / 左 / 下 / 后） |
| `Shift` + 上面任意键 | 逆时针 |
| **`S`** | 随机打乱 25 步 |
| **`Q`** | **求解复原**（Kociemba 两阶段，约 0.5 ms） |
| `Ctrl + R` | 按历史逆序复原（一步一步转回去） |
| `Esc` | 退出 |

> `Q` 第一次按会先建剪枝表（约 55~85 ms，只建一次），之后每次求解都是亚毫秒级。
