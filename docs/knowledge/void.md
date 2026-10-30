# `void` 类型：用法全景（含 `volatile`）

`void` 的核心定位：**一个不完整类型（incomplete type）——永远不会有对象、不能 `sizeof`、不能定义引用，但它作为"类型"真实存在于类型系统中**。它的家族成员各有明确用途，本文逐一介绍。

## 1. 函数返回类型：表示"不返回值"

```cpp
void f() { /* ... */ }   // 没有返回值

void g() {
    return;     // OK:提前结束
    // return 42;  // 错误:void 函数不能返回数值
}
```

细节：返回类型为 `void` 的函数里写 `return f();`（f 也是 void 函数）是合法的，等价于 `return;`。

## 2. 函数参数：C 与 C++ 的差异

```cpp
void f();      // C++:无参数。C:参数不明(不推荐)
void g(void);  // C 和 C++ 都表示"无参数"
```

C++ 里两者等价；写 `void` 是为了与 C 头文件兼容。

## 3. void 表达式：能存在，但几乎不能参与运算

调用 void 函数的表达式，其类型就是 `void`：

```cpp
void f();
f();            // OK:作为表达式语句执行
int x = f();    // 错误:不能初始化变量
f() + 1;        // 错误:void 不能作运算符的操作数
(f(), 42);      // OK:逗号运算符的左操作数允许是 void
```

void 表达式只能出现在极少数上下文：表达式语句、逗号运算符左侧、`return;`。这就是 `void` 的"值语义"——**没有值**。

## 4. 显式丢弃值：`static_cast<void>`

```cpp
int unused = compute();
static_cast<void>(unused);   // 明确告诉编译器和人:"我故意不用它"
```

在模板代码里常用（比如"这个返回值可能存在但我不关心"），比 `(void)x;` 的 C 风格写法更符合 C++ 习惯。

## 5. `void*`：void 最重要的用途——通用对象指针

`void*` 是"指向未知类型对象的指针"，是类型擦除的基础：

```cpp
int i = 42;
void* p = &i;                       // 任何对象指针都能隐式转 void*
int* back = static_cast<int*>(p);   // 转回原类型,由程序员保证正确
```

规则要点：

- **不能解引用**：`std::cout << *p;` 编译错误（不知道所指对象的大小和类型）；
- **不能算术运算**：`p + 1` 错误（不知道步长）；
- 只能做的事：赋值、比较、**转回具体类型**。

典型用途：

```cpp
// C API 风格:数据走 void*,类型信息靠另一个参数或调用约定
void* slot = nullptr;             // 存任何东西
slot = &some_object;
auto* rp = static_cast<Payload*>(slot);   // 取用时记得原类型
```

`memcpy`、`malloc`/`free`、POSIX 线程库的 `void* arg`、窗口系统的 user data，全是这个模式。

## 6. `const void*`：只读的擦除视图

```cpp
void* p = &i;
const void* q = p;   // OK:隐式加 const
void* r = q;         // 错误!丢失 const,需要 const_cast<void*>(q)
```

标准库的 `memcpy(dst, src, n)` 第二参数就是 `const void*`——"我要读这块内存，但承诺不修改"。

## 7. `volatile void*`：裸内存访问

`volatile` 告诉编译器"这块内存的内容可能在我看不见的时候变，每次访问都真的去读/写，不要优化掉"。配上 `void*`，就是嵌入式和底层代码访问任意内存地址的惯用写法：

```cpp
// 把物理地址上的数据搬到缓冲区:源和目标的"类型"对硬件毫无意义
void read_block(volatile void* dst, const volatile void* src, unsigned n) {
    auto* d = static_cast<volatile unsigned char*>(dst);
    auto* s = static_cast<const volatile unsigned char*>(src);
    for (unsigned i = 0; i < n; ++i) d[i] = s[i];  // 每次循环都真实读写
}
```

内存映射寄存器（`0x40000000` 之类的地址）、DMA 缓冲区、多进程共享内存，都是这个套路。**注意 `volatile` 不是线程同步工具**，它只管"别让编译器优化掉访问"，不保证原子性和内存序。

## 8. cv 限定写在 void 本身：`const void` / `volatile void`

`void` 没有对象，cv 限定本身不产生效果——允许写出来纯粹是为了类型系统统一：

- 模板推导真的会得到 `const void`：`template <class T> void f(T*);` 传入 `const void*` 时，T 推导为 `const void`；
- cv 规则对所有类型一致，泛型代码（如 `std::add_const_t<T>`）不用为 void 特判；顺带一提，标准库特意规定 `std::add_const_t<void>` 的结果是 `void` 而非 `const void`，避免意外。

日常代码里你不会主动声明 `const void` 的变量——也声明不出来，因为没有对象。详见讨论：为什么 void 还要分 const 和 volatile。

## 常见坑

- 解引用或算术运算 `void*`：

```cpp
void* p = &i;
*p;    // 错误
p + 1; // 错误
```

- `void*` 只能保存**对象指针**，不能存函数指针（两者不互通）：

```cpp
void* p = &some_function;              // 错误
auto fp = reinterpret_cast<void(*)()>(&some_function);  // 函数指针要用函数指针类型
```

- `volatile` 不是多线程同步工具，别拿它替代 `std::atomic` 或互斥锁。
- C++ 里对象指针转 `void*` 用 `static_cast`；不要习惯性地用 `reinterpret_cast` 包一层。
- 返回 `void` 的函数之间 `return f();` 合法，但 `void` 函数 `return 42;` 是错误。

## 小结

| 形式 | 含义 | 典型场景 |
|---|---|---|
| `void f()` | 函数不返回值 | 普通函数 |
| `f()` 表达式 | 类型为 void 的表达式 | 只作语句，不能参与运算 |
| `static_cast<void>(x)` | 显式丢弃值 | 模板/抑制未使用警告 |
| `void*` | 指向未知类型对象的指针 | 类型擦除、C API、`malloc` |
| `const void*` | 只读擦除视图 | `memcpy` 参数 |
| `volatile void*` | 需真实读写的裸内存 | 寄存器、DMA、共享内存 |
| `const void`（类型本身） | 无对象，cv 空转 | 仅为类型系统一致性 |

核心记法：`void` = "没有"；`void*` = "有，但类型先不告诉你"。前者用于函数语义，后者用于数据擦除——这是 void 家族的两大支柱。
