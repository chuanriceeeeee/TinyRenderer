# time complexity
- O(n)
  - 不是O(nlog(n)),因为对于每个像素只是在求最小值,n个遮盖,每个遮盖比较一次
# std::vector<double> zbuffer;
- 采用全局变量，在main中extern定义
```c++
extern std::vector<double>
```

