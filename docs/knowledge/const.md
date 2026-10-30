# `const int*` 与 `int* const`：顶层 const 与底层 const

一句话区别：**const 锁的东西不同**——`int *const` 锁指针自己（不能改指向），
`const int*` 锁指向的对象（不能改值）。指针变量有两层：箭头本身（存的地址）
和箭头指向的格子（对象的内存），const 可以只锁其一。

## 1. 读法规则：从右往左，const 绑左边

声明从右往左念；`const` 默认绑定它**左边**的东西，左边没东西时才绑右边：

```cpp
int *const p;       // p 是 const 的指针，指向 int —— const 左边是 *,锁指针
const int *q;       // q 是 指针，指向 const int —— const 左边没东西，绑右边，锁对象
const int *const r; // 两层都锁：const 指针指向 const int
```

## 2. 行为对照

```cpp
int x = 1, y = 2;

int *const p = &x;
//  p ═══════► x        箭头焊死
p = &y;     // 错:assignment of read-only variable 'p'
*p = 42;    // 对:x 本身没锁

const int *q = &x;
//  q ───────► x🔒      对象上锁
q = &y;     // 对:箭头自由
*q = 42;    // 错:assignment of read-only location '*q'
```

| 写法 | 能改指向？ | 能改指向的值？ | const 锁的是 |
|---|---|---|---|
| `int*` | ✅ | ✅ | 无 |
| `int *const` | ❌ | ✅ | 指针自身（顶层） |
| `const int*` | ✅ | ❌ | 指向的对象（底层） |
| `const int *const` | ❌ | ❌ | 两层 |

## 3. 顶层 const 在类型系统里的特殊待遇

顶层 const 描述的是"变量自己"，因此：

- **按值传参/按值返回会剥掉它**：`void f(int)` 和 `void f(const int)`
  是同一个函数（参数顶层的 const 不参与函数类型）；而 `const int*`
  的底层 const 参与类型，`f(int*)` 与 `f(const int*)` 是两个重载。
- **模板实参按值传递时同样剥掉**：`is_const_v<int* const>` 为 true，
  但把 `int* const` 传给 `template<class T> void g(T)` 后，`T` 将是
  `int*`——顶层 const 没了，底层 const 永远保留。

## 4. 与 trait 的关系：`is_const` 只报顶层

`c20_stl::is_const` 检查的是类型**最外层**（对象自身）的 const：

```cpp
static_assert( c20_stl::is_const_v<int *const>);   // true :对象自身是 const 指针
static_assert(!c20_stl::is_const_v<const int *>);  // false:对象是普通指针,const 在底层
```

底层 const 属于"指向类型"的一部分，不影响指针对象自身的可赋值性，
所以 `is_const` 不算数。要探测底层 const，得对"去掉一层指针之后的
类型"再调 `is_const`——这正是 `remove_pointer` + `is_const` 组合的
经典用途，也是后续 trait 的积木玩法。
