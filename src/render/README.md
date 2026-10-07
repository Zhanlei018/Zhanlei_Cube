# cube/render —— 渲染层

职责：网格生成、着色器封装、轨道相机。这是唯一大量接触 GLM 的地方。

## 要写的东西

| 文件 | 内容 |
| --- | --- |
| `mesh.hpp/.cpp` | 程序化生成 cubie 网格（24 顶点 / 面法线独立）；VAO / VBO 封装 |
| `shader.hpp/.cpp` | 从字符串或文件编译着色器；uniform 辅助；**编译失败时必须打印 infoLog** |
| `camera.hpp/.cpp` | 球坐标轨道相机 `(yaw, pitch, radius)`，输出 view / projection 矩阵 |

## 初始配色约定（自己定死，写进注释）

```
+X 红   -X 橙
+Y 白   -Y 黄
+Z 绿   -Z 蓝
```

世界坐标范围 [-1.5, 1.5]，相机距离 6 ~ 8。

## 着色器

放在根目录 `shaders/`，用相对路径加载。建议在 Debug 下加一个 `R` 键热重载：
改完 glsl 按一下就能看效果，省掉无数次重启。

## 常见坑速查

- 忘记 `glEnable(GL_DEPTH_TEST)` → 画面像半透明
- pitch 不 clamp 到 ±89° → 相机翻转 / 万向锁
- 鼠标 Y 轴没翻转（屏幕 y 向下、世界 y 向上）
- 窗口 resize 没重算 projection → 魔方被拉扁
- 矩阵顺序：`projection * view * model`
