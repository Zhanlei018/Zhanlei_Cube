# cube/cube —— 逻辑模型（全项目的心脏）

职责：魔方状态、转动操作、状态提交。**零 OpenGL，零浮点。**

## 数据模型

```
Cubie { glm::ivec3 pos;   // 分量 ∈ {-1, 0, 1}
        glm::imat3 orient; } // 单位阵起步，只含 0 / ±1

State { std::array<Cubie, 26> cubies; }   // 8 角 + 12 棱 + 6 心
Move  { int axis;    // 0=X 1=Y 2=Z
        int layer;   // 0/1/2
        int dir;     // +1 / -1，右手定则
        int turns; }  // 1 或 2（2 表示 180°）
```

## 转动公式（只有两行）

```
新 pos    = R * 旧 pos
新 orient = R * 旧 orient      // 左乘，顺序不能反
```

层筛选：`pos[axis] == layerOf(...)`，即 `pos[axis] == layer - 1`。

## 必须提供的接口

- `applyMove(State&, const Move&)` —— 立即修改逻辑状态（动画结束时才调用一次）
- `isSolved(const State&) const` —— 六个面各自同色
- `print(const State&)` —— 六个面用字母打印，终端调试用
- `operator==(const State&, const State&)` —— 给测试用

## 建议的验证顺序

1. 单个 Move 转 4 次必须回到原状态
2. `R U R' U'` 连做 6 次必须回到原状态（经典周期测试）
3. 随机 25 步 + 逆序 25 步必须逐位相等
