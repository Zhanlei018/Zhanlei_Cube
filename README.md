# Cube —— 3x3x3 魔方（C++17 + OpenGL）

一个**边写边学**的项目。不是现成的魔方软件，而是一副搭好的脚手架，
外加一份写清楚"为什么这么做"的工程笔记。

> 脚手架只搭骨架和说明，**不含业务实现**。
> 每个 `.cpp` 都要你自己写 —— 这正是重点。

---

## 环境（本机已探测）

| 项 | 值 |
| --- | --- |
| 编译器 | MinGW-w64 `C:\mingw64\bin\g++.exe` |
| 构建 | CMake `C:\mingw64\bin\cmake.exe` + Ninja |
| 版本控制 | Git `C:\Program Files\Git\cmd\git.exe` |
| 依赖 | GLFW 3.4 / GLM 1.0.1 / GLAD v2.0.8（**FetchContent 自动拉取**） |

没有 MSVC，没有 vcpkg —— 也不需要，CMake 会自己把依赖下到 `build/` 里。

> **网络提示**：首次 configure 要从 `github.com` 拉三个依赖。
> 大陆网络直连会超时（`Failed to connect to github.com port 443`），
> 开代理/加速器即可。依赖只拉一次，之后走 `build/` 缓存，日常编译不再联网。

---

## 快速开始

```powershell
cd D:\Cube

# 第一次 configure 要联网拉依赖，会慢一点
cmake --preset debug
cmake --build --preset debug

# 跑逻辑测试（现在它还是空的，等你写）
ctest --preset debug
```

`configure` 结束时会打印一份进度表，告诉你哪些文件已就位、哪些还没写：

```
  Cube 工程进度
  --------------------------------
    [skip] src/math/math.cpp  <- 还没写
    [skip] src/cube/state.cpp  <- 还没写
    ...
```

**先读 [NOTES.md](NOTES.md)**，那里有坐标约定、分区规则和七阶段的验收标准。

---

## 目录结构

```
D:\Cube
├── CMakeLists.txt          构建脚本（含进度守卫，缺文件也能编）
├── CMakePresets.json       debug / release 两套预设
├── NOTES.md                ★ 工程笔记：约定、路线图、坑位清单
├── README.md               本文件
│
├── shaders/                .vert / .frag（建议加 R 键热重载）
│
├── src/
│   ├── math/        薄封装：整数旋转矩阵、格坐标 ↔ 层号
│   ├── cube/        ★ 逻辑模型：Cubie / State / Move / applyMove
│   ├── render/      程序化网格、着色器、轨道相机
│   ├── input/       射线拾取 + 拖动方向解析（最易翻车）
│   ├── anim/        转动动画队列
│   ├── solver/      打乱、历史逆序复原、逐层法求解
│   └── app/         main + 输入状态机 + 主循环
│
└── tests/           纯逻辑单元测试（不链接任何图形库）
```

每个模块目录里都有一个 `README.md`，写清了**该写什么、接口长什么样、常见坑在哪**。

---

## 三个架构铁律

1. **`src/cube/` 永远不许 include OpenGL / GLFW 头文件。**
   魔方转得对不对，必须能在终端里、不看窗口的情况下验证。

2. **逻辑状态只在动画结束那一帧提交一次。**
   渲染循环里绝不按当前动画角度去改逻辑状态。

3. **所有输入最终都归一化成同一个 `Move { axis, layer, dir }`。**
   鼠标拖动、键盘、打乱、复原，走的都是同一条管线。

---

## 推荐动手顺序

| # | 阶段 | 产出 | 验收标准 |
| --- | --- | --- | --- |
| 1 | 状态表示 | `math/` + `cube/` | 终端输入 Move 序列，转 4 次回原位；打乱+逆序逐位相等 |
| 2 | 渲染 + 轨道相机 | `render/` | 26 个小方块正确显示，鼠标拖背景能转视角，不透明、不被拉扁 |
| 3 | 射线拾取 | `input/picking` | 悬停的小方块高亮，边角处也不偏 |
| 4 | 拖动 → Move | `input/drag` | 六个面各拖一次，方向都符合直觉 |
| 5 | 动画队列 | `anim/` | 转动平滑，动画期间连点不丢步、不串位 |
| 6 | 打乱 + 逐步复原 | `solver/` 前半 | 打乱 25 步，按 Z 一步一步真实转回去 |
| 7 | 任意状态求解 | `solver/` 后半 | 逐层法；先终端调通再接 UI |

**阶段 1 和阶段 3 是两个最重要的里程碑。** 前者保证逻辑对，后者保证坐标系统全对。
这两个过了，剩下的都是体力活。

---

## 调试热键（建议实现）

| 键 | 作用 |
| --- | --- |
| `S` | 随机打乱 |
| `Z` | 逆序复原 |
| `P` | 终端打印逻辑状态 |
| `H` | 悬停高亮开关 |
| `D` | debug 覆盖层（显示算出的 axis / layer / sign） |
| `R` | 重载着色器 |
| `Esc` | 退出 |

`D` 这个覆盖层会在阶段 4 救你一整天。
