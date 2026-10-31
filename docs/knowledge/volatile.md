# `volatile` 的用法

`volatile` 的核心语义只有一句话：**告诉编译器"这个对象的值可能在我看不见的时刻被改变"，因此每次读写都必须真实地访问内存，禁止优化掉、禁止缓存进寄存器。**

## 1. 语义实证：它到底阻止了什么优化

用同一段逻辑对比普通变量和 volatile 变量（`-O2`）：

```cpp
int gi;
volatile int gvi;

int read_plain()    { return gi + gi; }    // 读普通变量两次
int read_volatile() { return gvi + gvi; }  // 读 volatile 变量两次
```

反汇编（节选）：

```asm
read_plain:
    movl    gi(%rip), %eax     ; 只加载一次
    addl    %eax, %eax         ; eax + eax
read_volatile:
    movl    gvi(%rip), %eax    ; 加载两次!
    movl    gvi(%rip), %edx
    addl    %edx, %eax
```

普通变量的两次读被合并成一次（假设中间没人改它）；volatile 变量的两次读**一次都不能省**——因为硬件随时可能改写它。

补充两个精确的保证边界：

- C++11 起，**同一线程内** volatile 访问之间保持顺序，不会被编译器重排；
- 但 volatile **不提供任何跨线程的同步语义**（详见第 6 节）。

## 2. 两大合法用途

### 2.1 内存映射硬件（MMIO）

嵌入式编程中，硬件寄存器映射在固定地址上，程序看不见的时刻硬件会改写它的值：

```cpp
// 状态寄存器:每次读都必须真实发生,否则读到的是过期的缓存值
volatile unsigned* status = reinterpret_cast<volatile unsigned*>(0x40000000);
while (!(*status & READY_BIT)) { /* 忙等硬件就绪 */ }
```

把 `volatile` 去掉，编译器可能把循环优化成"只读一次寄存器，然后拿缓存值死循环"。

### 2.2 信号处理程序中的共享变量

信号处理程序可能在任何时刻执行，修改普通变量对主程序的优化不可见。C++ 标准要求这类标志用 `volatile std::sig_atomic_t`：

```cpp
#include <csignal>

volatile std::sig_atomic_t g_stop = 0;

void on_signal(int) { g_stop = 1; }

int main() {
    std::signal(SIGINT, on_signal);
    while (!g_stop) { /* 主循环;每次循环都真实读取 g_stop */ }
}
```

注意：这套机制只适用于**信号处理程序**这种"同一线程内被异步打断"的场景，不是通用多线程方案。

## 3. 类型系统视角：volatile 限定的是谁？

和 `const` 完全对称，四种写法含义不同：

```cpp
volatile int vi = 0;
int other = 1;

volatile int* p1 = &vi;   // 所指对象易变(底层 volatile)
p1 = &other;              // OK:p1 自身不是 volatile,可照常改指向

int* volatile p2 = &other;  // 指针本身易变(顶层 volatile)
*p2 = 42;                   // 所指对象的访问不特殊
p2 = nullptr;               // OK:volatile 不影响语法,只影响对 p2 值的优化

volatile int* volatile p3 = &vi;  // 所指对象和指针本身都易变
```

记忆法：和 `const` 读法一模一样——`volatile int*` 是"指向易变 int 的指针"，`int* volatile` 是"易变的 int 指针"。

一个类型系统细节：**顶层 volatile 在按值传递时被忽略**：

```cpp
void f(int x) {}
volatile int vx = 0;
f(vx);   // OK:拷贝发生时顶层 volatile 被剥掉(与顶层 const 同理)
```

## 4. volatile 成员函数

`volatile` 限定符参与成员函数重载，规则与 `const` 完全对称：

```cpp
struct S {
    int get()                { return 0; }
    int get() const          { return 1; }
    int get() volatile       { return 2; }
    int get() const volatile { return 3; }
};

S s;                  s.get();   // 0
const S cs;           cs.get();  // 1
volatile S vs;        vs.get();  // 2
const volatile S cvs; cvs.get(); // 3
```

规则：volatile 对象只能调用 volatile（或 const volatile）成员函数，非 volatile 对象四个版本都能调，按上面的优先级取最匹配者。

## 5. `const volatile`：只读寄存器

两者组合描述一种常见硬件："程序不许写它，但每次读都必须真实发生"——典型的只读状态寄存器：

```cpp
struct Reg {
    const volatile unsigned status;   // 只读 + 不可缓存
};
// Reg* r = 硬件地址;
// while (!(r->status & READY)) ...   // 每次循环真实读取
```

## 6. volatile 不是线程同步工具（最重要）

C++11 内存模型之后，这一点反复强调都不为过：

| 需求 | volatile | std::atomic |
|---|---|---|
| 防止编译器优化掉访问 | ✅ | ✅ |
| 同一线程内访问保序 | ✅ | ✅ |
| 操作原子性（不会读到写一半的值） | ❌ | ✅ |
| 跨线程可见性 / happens-before | ❌ | ✅ |

经典错误：一个线程写标志、另一个线程读标志——`volatile bool flag` 做不到安全同步，必须用 `std::atomic<bool>`：

```cpp
// 错误:数据竞争,UB
volatile bool ready = false;

// 正确
#include <atomic>
std::atomic<bool> ready{false};
```

`volatile` 也不保证 CPU 层面的内存顺序（缓存、乱序执行），那些是 `std::atomic` 配合 memory order 负责的领域。

## 常见坑

- **拿 volatile 当线程同步**：见第 6 节，这是第一大误用。
- **const 与 volatile 重载并存时的歧义**：只提供 `get() const` 和 `get() volatile` 两个版本时，volatile 对象调用 `get()` 会**编译报错（二义性）**，必须补 `get() const volatile`。
- **误以为 volatile 访问是原子的**：`volatile int` 的自增仍是"读-改-写"三步，多线程下照样丢更新。
- **和 `const` 混淆限定位置**：`volatile int*` 与 `int* volatile` 是两个不同的类型。
- **以为 volatile 能阻止 CPU 乱序**：它只管编译器层面；硬件层面的顺序由 `std::atomic` 的内存序保证。

## 小结

| 写法 | 含义 |
|---|---|
| `volatile int x` | x 的每次读写都必须真实发生 |
| `volatile int* p` | 所指对象不可优化（p 自身可优化） |
| `int* volatile p` | p 自身的值不可优化（所指对象照常） |
| `void f() volatile` | volatile 对象可调用的成员函数 |
| `const volatile` | 只读且不可缓存（硬件寄存器） |
| 线程同步 | ❌ 用 `std::atomic` |
