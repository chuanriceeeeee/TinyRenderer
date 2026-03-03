# c++禁止临时变量作为普通引用绑定
```c++
void doubleValue(int& x) {
    x = x * 2;
}

int main() {
    int a = 5;
    int b = 3;
    
    // 假设 C++ 允许这行代码编译通过：
    doubleValue(a + b); // 灾难现场！
    
    return 0;
}
```

```c++
void doubleValue(const int& x) {
    x = x * 2;
}

int main() {
    int a = 5;
    int b = 3;
    
    doubleValue(a + b);    
    return 0;
}
```