# 头文件里的变量为什么要加 `inline`

本文解释：为什么 `is_same_v` 这类定义在头文件里的变量，必须写成
`inline constexpr`。结论可以一句话说完——**`inline` 允许同一个变量定义
出现在多个翻译单元中，链接器负责合并**——但值得把来龙去脉讲清楚，
因为这个关键字是 C++ 里被误解最深的名字之一。

## 1. 从一个报错说起

假设头文件里定义了一个普通变量（没有 `inline`、没有 `constexpr`）：

```cpp
// counter.hpp
#ifndef COUNTER_HPP
#define COUNTER_HPP

int g_counter = 0;   // 问题就出在这里

#endif
```

两个源文件都包含它：

```cpp
// a.cpp
#include "counter.hpp"
void bump_a() { ++g_counter; }

// b.cpp
#include "counter.hpp"
void bump_b() { ++g_counter; }

// main.cpp
#include "counter.hpp"
int main() { bump_a(); bump_b(); }
```

编译各自通过，**链接时报错**：

```
ld: duplicate symbol 'g_counter' in:
    a.o
    b.o
```

每个 `.cpp` 文件各自编译成一个 `.o`（称为一个**翻译单元**，translation
unit），每个翻译单元都照着头文件生成了一份 `g_counter`。链接器看到
多个同名的强符号，不知道该留谁——直接拒绝。

这就是 **ODR（One Definition Rule，单一定义规则）**：程序中的非 inline
变量在整个程序里只能有**一次定义**。头文件会被无数 `.cpp` 包含，天然
违反这条规则。

## 2. 函数的同款问题，和 C++98 的解法

ODR 并不是变量的专利。早在 C++98，"头文件里写函数定义"就遇到同样的
事：

```cpp
// utils.hpp
int max2(int a, int b) { return a > b ? a : b; }   // 头文件里的函数定义
```

两个 `.cpp` 都包含它 → `duplicate symbol 'max2'`。

当年的解法就是 `inline`：

```cpp
inline int max2(int a, int b) { return a > b ? a : b; }
```

`inline` 在这里的语义**不是**"请编译器把这个函数内联展开"，
而是一个针对**链接器**的指令：

> 这个函数的定义可能出现在多个翻译单元中，它们彼此完全相同，
> 你（链接器）随便挑一份保留、丢弃其余的，不要报错。

讽刺的是名字起得极具误导性——它的真实含义是"**允许重复定义，由链接器
合并**"，和内联优化几乎无关。编译器是否内联展开这个函数，由它自己的
优化决策决定，和写不写 `inline` 基本无关。

## 3. C++17：规则扩展到变量

头文件里的函数问题在 1998 年就解决了，但**变量**一直没有对等的机制，
一等就是 19 年。直到 C++17：

```cpp
template <typename T, typename U>
inline constexpr bool is_same_v = is_same<T, U>::value;
// ^^^^^ 允许这个定义出现在多个翻译单元，链接器合并
```

规则与函数版一字不差：多个翻译单元里的 `inline` 变量定义被视为同一份，
链接器合并成**唯一一个实体**——所有 `.cpp` 读到的是同一个变量，地址
也相同。这就是我们要的效果。

## 4. `constexpr` 和 `inline` 是什么关系

你可能会想：变量都 `constexpr` 了，编译期常量，还有链接问题吗？

分两个时代看：

- **C++17 之前**：`constexpr` 命名空间作用域变量默认是**内部链接**——
  每个翻译单元拿到自己独立的一份拷贝。这确实绕开了重复定义报错，
  但代价是程序里存在多个 `g_counter` 拷贝，**取地址会发现它们不相等**，
  行为怪异且浪费。
- **C++17 起**：标准委员会做了正确的修复——**`constexpr` 变量隐含
  `inline`**。一份定义、外部链接、全程序唯一，地址唯一。

所以严格说，你的 `is_same_v` 只写 `constexpr` 也能链接通过。但标准库
和通行写法都是显式 `inline constexpr`，理由有二：

1. **表达意图**：`inline` 声明"这个定义允许跨翻译单元共享"，是接口
   层面的承诺，不该靠 `constexpr` 的副作用顺带获得。
2. **概念独立**：`constexpr` 管"何时求值"，`inline` 管"如何链接"，
   两件正交的事用两个关键字各管各的，读代码的人不用背隐含规则。

## 5. 亲手复现报错（建议做一次）

在 `build/` 之外的临时目录做这个小实验，比读十遍文档深刻：

```cpp
// t.hpp
#ifndef T_HPP
#define T_HPP
bool flag = true;            // 故意不加 inline、不加 constexpr
#endif
```

```cpp
// x.cpp
#include "t.hpp"
bool get_x() { return flag; }

// y.cpp
#include "t.hpp"
bool get_y() { return flag; }
```

```bash
g++ -std=c++20 -c x.cpp
g++ -std=c++20 -c y.cpp
g++ x.o y.o -o demo
# ld: duplicate symbol 'flag' ...
```

把 `bool flag = true;` 改成 `inline bool flag = true;`，重新走一遍——
链接成功。然后给 `flag` 加一句 `static_assert(flag);` 的变体练习：
体会"加不加 `inline` 都和编译期求值无关，只和链接有关"。

## 6. 常见误解清算

| 误解 | 事实 |
|---|---|
| `inline` 让代码更快 | 对变量：与性能无关。对函数：内联决策由编译器优化级别决定 |
| `inline` 让函数/变量"内联进调用点" | `inline` 只约束**链接行为**（允许多处定义、链接器合并） |
| `constexpr` 变量天然没有链接问题 | C++17 起才成立（constexpr 隐含 inline），且显式写更诚实 |
| 头文件里的变量加 `static` 也行 | 那是内部链接——每个翻译单元一份拷贝、地址不同，是旧时代的 workaround，不是解法 |

## 7. 实践准则

1. **头文件里的变量定义** → `inline`（配合 `constexpr` 写全
   `inline constexpr`）。
2. **头文件里的函数定义**（非模板） → `inline`；模板函数天然豁免，
   不需要。
3. **只在单个 `.cpp` 里用的变量** → 什么都不用加，加 `inline`
   反而制造了虚假的共享语义。
4. 看到链接报 `duplicate symbol`，第一反应：某个头文件里的定义
   缺了 `inline`。

## 8. 回到 `is_same_v`

```cpp
template <typename T, typename U>
inline constexpr bool is_same_v = is_same<T, U>::value;
```

- `inline`：这个变量定义会出现在每个包含头文件的翻译单元里，
  请链接器合并成一份。
- `constexpr`：值在编译期确定，可用于 `static_assert` / `if constexpr`。
- 两个关键字各管一件事，缺一不可，分工正是本文的主题。
