# triangle area
```c++
double triangle_area(parameters)
{
    //
    return .5 * ((yb - ya) * (xb + xa) + (yc - yb) * (xc + xb) + (ya - yc) * (xa + xc));
}
```

# barycentric coordinates
```c++
double alpha = triangle_area(xa, ya, xb, yb, x, y) / total_area;
double beta = triangle_area(xb, yb, xc, yc, x, y) / total_area;
double gamma= triangle_area(xc, yc, xa, ya, x, y) / total_area;
```

- $\alpha, \beta ,\gamma $
- $$\alpha = \frac{\text{Area}(PBC)}{\text{Area}(ABC)}$$
- $$\beta = \frac{\text{Area}(PCA)}{\text{Area}(ABC)}$$
- $$\gamma = \frac{\text{Area}(PAB)}{\text{Area}(ABC)}$$
>三者分别代表点距离 A,B,C的距离等于1时距离对应点最大

>三者相加等于1

>在三角形外部的点为负数

# determine whether a point outside a triangle
- 扩展方法 1：叉乘同向法 / 半空间测试（Edge Equation / Cross Product）—— 现代 GPU 的最爱 
  - 这是现代硬件光栅化器（比如你电脑里的 Nvidia 显卡）真正在用的底层方法。
  - 原理：把三角形的三条边看作有方向的向量：$\vec{AB}$, $\vec{BC}$, $\vec{CA}$。
  - 对于测试点 $P$，我们分别计算每条边和 $P$ 构成的向量的二维叉乘（2D Cross Product）。
    - 计算 $\vec{AB} \times \vec{AP}$
    - 计算 $\vec{BC} \times \vec{BP}$
    - 计算 $\vec{CA} \times \vec{CP}$
  - 二维叉乘的结果是一个标量（有正有负）。判断依据：如果这三个结果符号完全相同（全为正，或全为负，取决于顶点是顺时针还是逆时针），说明点 $P$ 在三条边的同一侧，即在三角形内部。如果有正有负，说明点 $P$ 在外部。
- 扩展方法 2：射线法（Ray Casting / Even-Odd Rule）—— 适用于任意多边形不仅是三角形，这个方法可以判断点是否在**任何奇形怪状的多边形（比如凹多边形）**内部。
  - 原理：从点 $P$ 出发，向任意方向（通常是水平向右）发射一条射线。数一数这条射线和多边形的边界相交了几次。
  - 判断依据：交点个数为奇数（1, 3, 5...）：点 $P$ 在内部。交点个数为偶数（0, 2, 4...）：点 $P$ 在外部。(想象你站在一个圈里往外走，你必定会穿过边界奇数次才能彻底跑出去)。
- 扩展方法 3：内角和法（Angle Sum）—— 面试聊聊就好，工程千万别用
  - 原理：连接 $P$ 到 $A, B, C$，算出夹角 $\angle APB$, $\angle BPC$, $\angle CPA$。
  - 如果点在内部，这三个角的和等于 360°。
  - 如果点在外部，这三个角的和会小于 360°。
  >为什么不用它？ 因为计算角度需要用到反三角函数（acos），这在 CPU/GPU 里极其消耗性能，对于动辄几百万次循环的光栅化来说是灾难。

# full-color triangle rasterization
```c++
//RGBA order. If BGRA, need to change the order.
TGAColor color = {static_cast<unsigned char>(alpha * 255),static_cast<unsigned char>(beta * 255),static_cast<unsigned char>(gamma * 255)}
```

# MVP
- 从模型空间到屏幕空间的变换
- 
  - Model transform
    - 模型变换
    - 将原本位于世界原点的模型放大、旋转到世界的某个位置
  - View transform
    - 观察变换
    - 为了将摄像机位置置于世界原点，将整个世界反向移动，且让摄像机看向-Z轴
  - Projection transform
    - 投影变换
    - 用一个平截头体（Frustum）把摄像机能看到的东西框起来，然后把这个不规则的框，用力压成一个边长为 2 的正方体（坐标范围 $[-1, 1]$）

# Viewport transform
  - 视口变换
  - 将镜头展示缩放到屏幕大小
  ```c++
  vec4 project (parameters)
  {
    return vec4{
        (x + 1.0) / 2.0 * width,
        (y + 1.0) / 2.0 * height,
        z,
        0
    };
  }

  ```