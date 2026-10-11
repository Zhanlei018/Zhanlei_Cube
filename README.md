# Cube —— 3x3x3 魔方（C++17 + OpenGL）

一个**边写边学**的项目：自己实现魔方逻辑、自己写渲染管线。

---

## 目录结构

```
D:\Cube
├── CMakeLists.txt          构建脚本（含进度守卫，缺文件也能编）
├── CMakePresets.json       debug / release 两套预设
├── README.md               本文件：目录结构 + 启动方式
├── NOTES.md                工程约定：坐标系、配色表、状态不变量
│
├── shaders/                cube.vert / cube.frag
│
├── src/
│   ├── math/         薄封装：整数旋转矩阵、格坐标 ↔ 层号
│   ├── cube/         ★ 逻辑模型：Cubie / State / Move / applyMove
│   ├── render/       程序化网格、着色器、轨道相机
│   ├── input/        射线拾取 + 拖动方向解析（未开始）
│   ├── anim/         转动动画队列（未开始）
│   ├── solver/       打乱、历史逆序复原、逐层法求解（未开始）
│   └── app/          main + 主循环
│
├── tools/            有独立 main() 的验证程序
│   ├── math_check.cpp      数学层回归测试
│   ├── cube_check.cpp      逻辑层回归测试
│   └── winding_check.cpp   三角形绕序检查
│
├── tests/            纯逻辑单元测试
│
└── third_party/khr/  离线依赖：khrplatform.h（GLAD 生成时需要）
```

> 开发笔记（环境搭建、踩坑记录、阶段路线图）保留在本地 `DEVELOPMENT.md`，不进仓库。

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
cmake --build --preset debug --target cube_app     # 魔方程序
cmake --build --preset debug --target cube_check   # 逻辑层回归测试
cmake --build --preset debug --target math_check   # 数学层回归测试
```

### 回归测试

```powershell
.\build\debug\cube_check.exe    # 逻辑层：全部通过才算正常
.\build\debug\math_check.exe    # 数学层
```

`configure` 时会打印一份进度表，告诉你哪些文件已就位、哪些还没写：

```
  Cube 工程进度
  --------------------------------
    [ok]   src/math/math.cpp
    [ok]   src/cube/state.cpp
    [skip] src/input/picking.cpp  <- 还没写
```

---

## 当前操作

| 操作 | 效果 |
| --- | --- |
| 鼠标左键拖动 | 绕魔方转视角 |
| 滚轮 | 推拉远近 |
| 窗口关闭按钮 | 退出 |

键盘操作（转动魔方、打乱、复原）**尚未实现**。
