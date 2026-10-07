# cube/app —— 组装层

职责：`main`、GLFW 回调、主循环、以及全局输入状态机。

## 需要写的东西

| 文件 | 内容 |
| --- | --- |
| `main.cpp` | GLFW 初始化 → 主循环 → 清理；挂回调 |
| `input_state.hpp/.cpp` | 鼠标按键/移动的统一状态机（Idle / Pressed / Dragging / Committed） |

## 主循环骨架（伪代码，自己补全）

```
initWindow();  initGL();  loadShaders();  buildMeshes();
State cube = makeSolvedState();

while (!shouldClose) {
    dt = clamp(now - last, 0, 0.05)
    pollEvents()
    camera.update(dt)          // 只在左键拖背景时旋转视角
    animator.update(dt)        // 推进动画，结束时提交逻辑状态
    inputState.update()        // 需要时才产生 Move 并入队
    glClear(...)
    draw(cube, animator.currentRotation())
    swapBuffers()
}
```

## 鼠标按键分工（建议）

- **左键拖空白处** → 轨道相机旋转视角
- **左键拖魔方表面** → 触发对应层的转动
- **滚轮** → 推拉相机距离

区分方法：mouse down 时先做一次拾取。命中了就是拧魔方，没命中就是转视角。

## 建议加的调试热键

| 键 | 作用 |
| --- | --- |
| `S` | 随机打乱 25 步 |
| `Z` | 逆序复原（一步一步转回去） |
| `P` | 在终端打印逻辑状态 |
| `H` | 开关悬停高亮 |
| `D` | 开关 debug 文字覆盖层（显示算出来的 axis / layer / sign） |
| `R` | 重载着色器 |
| `Esc` | 退出 |

**`D` 这个覆盖层会在阶段 4 救你一整天。** 符号错了别猜，看着数字调。
