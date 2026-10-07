# cube/tests —— 逻辑层单元测试

**只链接 `cube_core`，不碰 OpenGL。** 这就是分层设计的意义：
魔方转得对不对，不需要开窗口就能验证。

## 运行

```
cmake --build --preset debug
.\build\debug\cube_tests.exe
```

## 必须有的用例（按优先级）

1. `solve state + 任意 Move` → 26 个 cubie 的 pos 分量绝对值仍 ≤ 1
2. `U` 转 4 次 == 原状态
3. `U2` == `U` 转 2 次
4. `R U R' U'` 做 6 次 == 原状态（周期测试，能抓出朝向提交的符号错误）
5. 随机 25 步 + 逆序逆操作 → 逐位等于初始状态
6. `isSolved()` 在打乱后必须为 false，复原后必须为 true

第 4 条和第 5 条是最有价值的两个：**它们能在第一现场抓到"位置对了但朝向错了"这类 bug。**

## 断言风格

用最朴素的 `EXPECT_TRUE` / `EXPECT_EQ` 宏就行，别急着上 gtest / Catch2。
自己写 30 行测试框架，比配依赖省时间。
