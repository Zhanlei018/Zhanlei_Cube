# min2phase —— Kociemba 两阶段魔方求解器（第三方）

本目录是**第三方代码**，不是本项目写的。放进来是为了给魔方程序提供
"按 Q 键即时求解"的能力。

## 来源

- 原始 Java 版：<https://github.com/cs0x7f/min2phase> （作者 Chen Shuang）
- C++ 移植版：<https://github.com/lilborgo/min2phaseCXX> （作者 Borgo Federico）
  —— 本目录取自这个版本

## 为什么用它

自己写四阶段（Thistlethwaite）每阶段都要建一张"坐标 → 到目标子群的步数"
距离表，坐标空间动辄上百万。Debug 构建下建一次表要几十秒，按 Q 键求解
根本等不起。

min2phase 靠**对称性归约**把剪枝表按等价类存，表大小直接除以 16~324：

| 剪枝表 | 规模 |
|---|---|
| UDSliceTwistPrun | 495 × 2187/324 |
| UDSliceFlipPrun | 495 × 2048/336 |
| TwistFlipPrun | 2048 × 2187/324 |
| MCPermPrun | 24 × 40320/2768 |
| EPermCCombPPrun | 140 × 40320/2768 |

总内存约 1 MB。本机实测：`init()` 约 55 ms，单次求解平均 0.5 ms，解长 20~21 步。

## 本项目的改动

**算法本体一行未动。** 只做了一处裁剪：

- 上游 `src/min2phase.cpp` → 这里的 `api.cpp`：删掉了 `server()` / `stop()` /
  `webSearch()` 三个函数，因此不再 `#include "http.h"`，也就**不需要链接 pthread**。
  理由：那个 HTTP server 是给"把求解器当服务跑"用的，我们在魔方程序里本地求解
  用不到，而它会拖进 pthread 依赖。
- 相应地没有收录 `http.h` / `http.cpp`。

其余文件（`info.*` / `CubieCube.*` / `coords.*` / `Search.*` / `tools.*` / `tests.*`）
与上游一致。

## ⚠️ 使用注意：转向约定

min2phase 用的是**标准魔方记号**，而本引擎的 `Move.dir` 在正负两个坐标层上
方向不一致（实测：同一个 `dir=+1`，在 `layer==2` 的 R/U/F 是逆时针，
在 `layer==0` 的 L/D/B 是顺时针）。所以从解串转成 `Move` 时，
`layer == 2` 的要取反。这段逻辑在 `src/solver/twophase_solver.cpp` 的
`faceLetterToMove()`，那里有详细注释。

守门员：`tools/twophase_test.cpp` 的逐动作闭环。改方向相关代码后务必跑它。

（`R2` / `L2` 这类 180° 动作是自逆的，方向错了看不出问题；**必须用 90° 动作测**。）

## 许可

上游是 **GPL-3.0 与 MIT 双许可**，各文件头部保留了原文。

⚠️ 如果按 GPL-3.0 分发，本项目整体要按 GPL-3.0 走；如果只想 MIT，
请确认所有相关文件都按 MIT 条款使用。**这是使用者需要自己做的许可决定。**

## 接口速查

```cpp
#include <min2phase/min2phase.h>

min2phase::init();   // 建表，约 55 ms，只需一次

std::string facelets = "UUUUUUUUURRRRRRRRRFFFFFFFFFDDDDDDDDDLLLLLLLLLBBBBBBBBB";
uint8_t usedMoves = 0;
std::string solution = min2phase::solve(facelets, 21, 1000000, 0, 0, &usedMoves);
// solution 形如 "R2 U' F ..."，空格分隔
```

facelet 串顺序：`U(9) R(9) F(9) D(9) L(9) B(9)`，每面从上到下、从左到右。
颜色：`U=白 R=红 F=绿 D=黄 L=橙 B=蓝`。

**必须在 `solve()` 之前调用 `init()`**，否则返回错误码 `9`（`MISSING_COORDS`）。

本项目已经包好了，正常用这两个就够：

```cpp
#include "solver/twophase_solver.hpp"

cube::warmUpTwoPhase();
std::vector<cube::Move> solution = cube::solveTwoPhase(state);
```
