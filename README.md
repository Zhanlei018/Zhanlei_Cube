# Cube —— 3x3x3 魔方（C++17 + OpenGL）

一个**边写边学**的项目：魔方逻辑、渲染管线、射线拾取、拖动交互全部自己实现。

---

## 目录结构

```
D:\Cube
├── CMakeLists.txt          构建脚本（含进度守卫，缺文件也能编）
├── CMakePresets.json       debug / release 两套预设
├── README.md               本文件：目录结构 + 启动方式 + 操作说明
├── DEVELOPMENT.md          开发笔记（本地文件，不进仓库）
├── NOTES.md                工程约定：坐标系、配色表、状态不变量
│
├── shaders/                cube.vert / cube.frag
│
├── src/
│   ├── math/         薄封装：整数旋转矩阵、格坐标 ↔ 层号
│   ├── cube/         ★ 逻辑模型：Cubie / State / Move / applyMove
│   ├── anim/         转动动画队列（纯时序逻辑，不碰 OpenGL）
│   ├── solver/       打乱与求解（打乱待写，求解未开始）
│   ├── render/       程序化网格、着色器、轨道相机
│   ├── input/        射线拾取 + 拖动 → Move（含实时预览）
│   └── app/          main + 主循环 + 输入回调
│
├── tools/            有独立 main() 的验证程序
│   ├── math_check.cpp       数学层回归测试
│   ├── cube_check.cpp       逻辑层回归测试
│   ├── pick_check.cpp       射线拾取验证
│   ├── drag_check.cpp       拖动映射验证
│   ├── drag_layers.cpp      旋转层精确性验证
│   ├── drag_e2e.cpp         预览→动画交接验证
│   ├── scramble_check.cpp   打乱序列验证
│   └── winding_check.cpp    三角形绕序检查（开背面剔除前先跑它）
│
├── tests/            纯逻辑单元测试（不链接任何图形库）
│
└── third_party/khr/  离线依赖：khrplatform.h（GLAD 生成时需要）
```

---

## 启动方式

```powershell
cd D:\Cube
cmake --preset debug                # 首次需要联网拉依赖，会慢一点
cmake --build --preset debug        # 构建全部
.\build\debug\cube_app.exe          # 运行魔方
```

> ⚠️ **必须在 `D:\Cube` 目录下运行。**
> 着色器用的是相对路径 `shaders/cube.vert`，直接双击 `build\debug\cube_app.exe`
> 会因为工作目录不对而读不到着色器。

### 只构建某一个目标

```powershell
cmake --build --preset debug --target cube_app          # 魔方程序
cmake --build --preset debug --target cube_check        # 逻辑层回归测试
cmake --build --preset debug --target pick_check        # 拾取验证
cmake --build --preset debug --target scramble_check    # 打乱验证
```

### 回归测试

```powershell
.\build\debug\cube_check.exe        # 逻辑层：全部通过才算正常
.\build\debug\math_check.exe        # 数学层
.\build\debug\pick_check.exe        # 射线拾取
.\build\debug\drag_check.exe        # 拖动映射
.\build\debug\drag_layers.exe       # 旋转层精确性
.\build\debug\scramble_check.exe    # 打乱序列
```

`configure` 时会打印一份进度表，告诉你哪些文件已就位、哪些还没写：

```
  Cube 工程进度
  --------------------------------
    [ok]   src/math/math.cpp
    [ok]   src/cube/state.cpp
    [ok]   src/solver/solver.cpp
    ...
```

---

## 操作

| 操作 | 效果 |
| --- | --- |
| **鼠标左键拖动魔方表面** | 对应层跟着转，松手后落位 |
| **鼠标左键拖动背景** | 绕魔方转视角 |
| **滚轮** | 推拉远近 |
| **`R` `U` `F` `L` `D` `B`** | 转对应面（右/上/前/左/下/后） |
| **`Shift` + 上面任意键** | 逆时针 |
| **`Ctrl + R`** | 按历史逆序复原（一步一步转回去） |
| **`S`** | 随机打乱 25 步 |
| **`Esc`** | 退出 |

### 拖动的手感

- 拖动**不超过 60%** 就松手 → **取消**，那一层弹回原位
- 拖动**超过 60%** → 预览锁定在 90°，松手后动画补完剩余部分

---

## 分层

**依赖方向只能向下，不能横向或向上。**

```
        app  (窗口 / 主循环 / 输入回调)
         │
   ┌─────┼─────────┬──────────┐
 render  input    anim      solver
   └─────┼─────────┴──────────┘
         │
       cube   ← 纯逻辑
         │
       math
```

| 库 | 内容 | 能链接什么 |
| --- | --- | --- |
| `cube_core` | `math/` + `cube/` | 只有 GLM（整数类型） |
| `cube_anim` | `anim/` | cube_core + GLM |
| `cube_solver` | `solver/` | cube_core + GLM |
| `cube_gfx` | `render/` + `input/` | 上面全部 + GLFW + GLAD |
| `cube_app` | `app/` | 以上全部 |

**这把分层写进了构建系统** —— `cube_anim` 和 `cube_solver` 不链接 GLFW/GLAD，
所以在里面写 `gl*` 调用会直接编译失败。比靠自觉可靠。