# cube/anim —— 转动动画队列

职责：让逻辑上的"瞬间转动"看起来像手拧的。

## 结构

```
AnimMove { Move move; float elapsed, duration; }
std::deque<AnimMove> queue;
AnimMove* current = nullptr;
```

## 每帧流程

1. `current` 为空且队列非空 → 弹出队首
2. `t = clamp(elapsed / duration, 0, 1)`，缓动 `e = t*t*(3-2*t)`
3. 绘制时，属于该层的 cubie 额外左乘 `rotate(axis, e * angle)`
4. `t >= 1` → **调用一次 `applyMove(state, move)` 提交逻辑状态**，清空 current

## 铁律

- 逻辑提交**只在动画结束那一帧发生一次**。渲染循环里绝不允许按当前角度去改逻辑状态
- 动画期间接受新输入 → 直接入队，不打断
- 用真实时间差，不用固定 dt；单帧 dt 要 clamp（比如 0.05s），否则拖窗口后动画一次跳完
- 结束判断用 `>= 1.0f` 并 clamp，不要用 `== 1.0f`
- duration 建议 0.15 ~ 0.25 秒

## 可选优化

同一层连续同向的 Move 可以合并成一个 180°，手感更好。
