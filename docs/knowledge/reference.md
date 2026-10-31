# 左值引用与右值引用：绑定、移动和引用折叠

左值引用 `T&` 与右值引用 `T&&` 描述的是**引用类型**，左值与右值描述的是
**表达式**。理解这两组概念的区别，才能看懂移动语义以及后面的引用 traits。

## 1. 先分清：表达式类别和引用类型

```cpp
int value = 42;

int& lref = value;   // 左值引用类型
int&& rref = 42;     // 右值引用类型
```

可以先这样建立直觉：

- `value`、`lref` 这样的名字作为表达式出现时，是左值，代表一个稳定对象。
- `42`、`int{42}` 这样的临时结果，是右值。
- `std::move(value)` 也产生右值，表示允许把 `value` 当作可移动来源。

左值和右值首先是在描述**表达式**，不能仅凭变量名里有没有 `&` 或 `&&` 判断。

## 2. 左值引用 T&：给已有对象起别名

非 const 左值引用通常绑定可修改的左值：

```cpp
#include <cassert>

int main()
{
    int value = 10;
    int& ref = value;

    ref = 20;             // 修改的就是 value
    assert(value == 20);
}
```

非 const 左值引用不能绑定临时对象：

```cpp
// int& ref = 42;       // 错误：不能绑定临时对象
```

const 左值引用是常见的例外：它既能绑定左值，也能绑定右值，但不能通过它修改
所引用的对象。

```cpp
int value = 10;

const int& a = value;    // 合法：绑定左值
const int& b = 42;       // 合法：绑定临时对象
```

“const 左值引用能绑定右值”不会把它变成右值引用，`const int&` 仍是左值引用类型。

## 3. 右值引用 T&&：允许接管右值的资源

右值引用可以绑定临时对象，也可以绑定经 `std::move` 转换后的表达式：

```cpp
#include <string>
#include <utility>

std::string&& a = std::string("hello");

std::string source = "world";
std::string&& b = std::move(source);
```

它的意义不是“这个引用跑得更快”，而是让移动构造函数、移动赋值运算符等接口能够
识别一个可以接管资源的来源。

`std::move(source)` 只改变表达式的类别，真正的资源转移由随后调用的移动操作完成。

## 4. 最容易踩的坑：命名的右值引用仍是左值表达式

```cpp
int&& rref = 42;
```

`rref` 的**声明类型**是 `int&&`，但 `rref` 这个名字作为表达式出现时仍是左值，
因为它代表一个有名字、可以反复使用的对象。

用 `decltype` 可以直接验证：

```cpp
#include <type_traits>
#include <utility>

int&& rref = 42;

static_assert(std::is_same_v<decltype(rref), int&&>);
// 变量的声明类型：int&&

static_assert(std::is_same_v<decltype((rref)), int&>);
// 名字作为表达式：左值，因此 decltype 推导为 int&

static_assert(std::is_same_v<decltype((std::move(rref))), int&&>);
// std::move 后重新得到右值表达式
```

这里的双层括号是在让 `decltype` 检查表达式，而不是直接读取变量的声明类型。

因此，把右值引用继续交给移动操作时，通常还要写 `std::move`：

```cpp
std::string source = "hello";
std::string&& ref = std::move(source);

std::string target = std::move(ref);
```

## 5. std::move 本身不移动

`std::move` 本质上是一次到右值引用的转换。下面用移动次数验证：

```cpp
#include <cassert>
#include <utility>

struct probe {
    static inline int moves = 0;

    probe() = default;
    probe(probe&&) { ++moves; }
};

int main()
{
    probe value;
    probe&& ref = std::move(value);
    assert(probe::moves == 0);       // 只做了转换，没有构造新对象

    probe target = std::move(ref);
    assert(probe::moves == 1);       // 这里才调用移动构造函数
}
```

移动完成后，原对象仍处于有效状态，但除非接口另有保证，不应依赖它原来的值。

## 6. 引用折叠：写引用 traits 的关键规则

通过类型别名或模板参数给引用类型再次添加引用时，会发生**引用折叠**：

| 原类型 | 再添加 | 折叠结果 |
|---|---|---|
| `T&` | `&` | `T&` |
| `T&` | `&&` | `T&` |
| `T&&` | `&` | `T&` |
| `T&&` | `&&` | `T&&` |

记忆规则：**只要出现左值引用，结果就是左值引用；两边都是右值引用，结果才是
右值引用。**

```cpp
#include <type_traits>

using L = int&;
using R = int&&;

static_assert(std::is_same_v<L&, int&>);
static_assert(std::is_same_v<L&&, int&>);
static_assert(std::is_same_v<R&, int&>);
static_assert(std::is_same_v<R&&, int&&>);
```

这直接决定了引用转换 trait 的契约：

```text
add_lvalue_reference_t<int&&> → int&
add_rvalue_reference_t<int&>  → int&
```

另外，`void` 等特殊类型不能形成引用，不能简单认为任何 `T` 都能写成 `T&`。

## 7. 进阶提醒：模板里的 T&& 不一定是右值引用

如果 `T` 需要从函数实参推导，`T&&` 通常是**转发引用**，而不是普通的右值引用：

```cpp
template <typename T>
void f(T&& value);

int value = 10;

f(value);   // T 推导为 int&，参数折叠为 int&
f(42);      // T 推导为 int，参数为 int&&
```

`int&&`、已经确定的别名后面的 `&&` 不涉及这种推导，仍按普通右值引用和引用折叠
理解。转发引用用于完美转发，不能在当前场景简单套一句“`&&` 只能绑定右值”。

## 8. 生命周期与悬垂引用

局部引用直接绑定临时对象时，可以延长临时对象的生命周期：

```cpp
const int& a = 42;
int&& b = 42;
```

临时对象会存活到对应引用离开作用域。但不要把这个规则推广到所有场景：返回局部
对象或局部临时对象的引用，仍可能产生悬垂引用。

## 9. 小结

| 类型 | 直接绑定左值 | 直接绑定右值 | 主要用途 |
|---|---|---|---|
| `T&` | 可以 | 不可以 | 给已有对象起可修改的别名 |
| `const T&` | 可以 | 可以 | 只读访问，也能接收临时对象 |
| `T&&` | 不可以 | 可以 | 标识可接管资源的右值来源 |

最重要的三句话：

1. 左值、右值描述表达式；左值引用、右值引用描述类型。
2. 命名的右值引用变量作为表达式出现时仍是左值，`std::move` 本身也不执行移动。
3. 引用折叠中，左值引用优先；只有 `T&&` 与 `&&` 相遇才得到 `T&&`。
