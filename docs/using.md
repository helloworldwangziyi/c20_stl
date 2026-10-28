# `using` 的用法

`using` 在 C++ 中有几种完全不同的用法，本文逐一说明。

## 1. using 声明：引入单个名字

把某个名字直接引入当前作用域，之后可以省略限定符：

```cpp
#include <iostream>

using std::cout;   // 之后直接写 cout
using std::endl;

int main() {
    cout << "hi" << endl;   // 等价于 std::cout << "hi" << std::endl;
}
```

也可以在局部作用域中使用，只在当前块内生效：

```cpp
void f() {
    using std::cout;   // 只在这个函数体内有效
    cout << "hi\n";
}
```

## 2. using 指令：引入整个命名空间

```cpp
using namespace std;   // std 里所有名字都可以直接使用

cout << "hi\n";        // 等价于 std::cout
vector<int> v;         // 等价于 std::vector<int>
```

### 和 using 声明的区别

| 写法 | 引入范围 |
|---|---|
| `using std::cout;` | 只引入 `cout` 一个名字 |
| `using namespace std;` | 引入 `std` 中所有名字 |

### 注意事项

- **头文件里不要写 `using namespace std;`**，会污染所有包含该头文件的代码。
- `using namespace` 容易引起名字冲突，尤其是项目变大、引入多个第三方库时。
- 推荐做法：函数体内、.cpp 文件中按需使用；或者只用 using 声明引入需要的少数几个名字。

## 3. 类型别名（C++11 起）

```cpp
using Vec = std::vector<int>;        // 等价于 typedef std::vector<int> Vec;
using Callback = void(*)(int);       // 函数指针别名
using ll = long long;
```

### 对比 typedef

`typedef` 也能做别名，但有两处不如 `using`：

**① 可读性**：新名字在等号左边，从左往右读更自然。

```cpp
typedef void (*Callback)(int);   // 旧写法,要从中间往外读
using Callback = void(*)(int);   // 新写法,更清晰
```

**② 别名模板**：`typedef` 不支持模板，`using` 可以：

```cpp
template <typename T>
using VecT = std::vector<T>;     // 别名模板,typedef 做不到

VecT<int>    a;
VecT<double> b;
```

日常写代码时代码中应优先用 `using` 取代 `typedef`。

## 4. 类中的 using

### 4.1 继承构造函数

派生类可以直接继承基类的构造函数，省去转发构造的样板代码：

```cpp
struct Base {
    Base(int x, double y);
};

struct Derived : Base {
    using Base::Base;   // 直接继承 Base 的所有构造函数
    // 等价于自己写:
    // Derived(int x, double y) : Base(x, y) {}
};
```

注意：基类构造函数的默认实参会一并保留，所以 `Derived d(1);` 这种少传参的写法依然有效。如果派生类自定义了同签名的构造函数，优先使用派生类自己的版本。

### 4.2 引入重载集（解决名字隐藏）

这是类中 using 声明更常见的用途。派生类一旦定义了与基类同名的成员，会**隐藏**基类该名字的**所有重载**：

```cpp
struct Base {
    void f(int);
    void f(double);
};

struct Derived : Base {
    void f(int);   // 隐藏了 Base::f 的全部重载!
};

Derived d;
d.f(3.14);   // 调用的是 Derived::f(int),3.14 被截断成 3
```

用 `using` 声明把基类的**整个重载集**引入即可解决：

```cpp
struct Derived : Base {
    using Base::f;   // 引入 Base::f 的所有重载
    void f(int);     // 再补充/覆盖自己需要的版本
};

Derived d;
d.f(3.14);   // 正确调用 Base::f(double)
```

要点：`using Base::f;` 引入的是名字 f 的**全部重载**，而不是某一个函数。

### 4.3 调整成员访问权限

基类的成员在派生类中访问权限被收窄时，可以用 `using` 恢复：

```cpp
class Base {
protected:
    void f();
};

class Derived : private Base {
public:
    using Base::f;   // f 在 Derived 中重新变为 public
};
```

注意：不能用它突破原本就不可访问的成员（例如把基类 `private` 成员暴露成 `public` 是不行的）。

## 5. 惯用法：using + ADL（swap 技巧）

写泛型代码时，经常需要调用某个「可能有用户自定义版本」的操作，典型例子是 `swap`：

```cpp
#include <utility>

template <typename T>
void func(T& a, T& b) {
    using std::swap;   // 把 std::swap 带入查找范围
    swap(a, b);        // 不加 std:: 前缀,刻意依赖 ADL
}
```

原理：

- 直接写 `std::swap(a, b)` 会**强制**使用标准库的 swap，用户为自定义类型定义的版本不会被找到。
- 什么都不写而直接 `swap(a, b)`，对没有自带 swap 的类型会因为找不到声明而编译失败。
- 先 `using std::swap;` 再调用**不带限定符的** `swap`：ADL 找到用户自定义版本就优先用，否则回退到 `std::swap`。

这个「`using` + 无限定调用」的模式是标准库风格的泛型代码惯用法。

## 6. using enum（C++20）

把枚举的枚举值直接引入当前作用域，写 `enum class` 时不用每次都带类名：

```cpp
enum class Color { red, green, blue };

void f(Color c) {
    using enum Color;        // C++20
    if (c == red) { /* ... */ }    // 不用写 Color::red
}
```

普通（不带 class 的）枚举也可以用：

```cpp
enum Dir { up, down, left, right };

void g() {
    using enum Dir;
    auto d = up;   // 不用写 Dir::up
}
```

## 7. 模板相关进阶

### 7.1 从依赖基类引入嵌套类型

```cpp
template <typename T>
struct Derived : T {
    using typename T::value_type;   // 依赖基类的嵌套类型,using 声明也要加 typename
    value_type x;
};
```

### 7.2 参数包展开（C++17）

```cpp
template <typename... Mixins>
struct Foo : Mixins... {
    using Mixins::operator()...;   // 一次引入所有基类的 operator()
};
```

这在 mixin / CRTP 组合多个基类的场景很有用。

## 常见坑

- `using namespace` 不要出现在头文件的文件作用域。
- 派生类中 `using Base::f;` 引入的是整个重载集；若与派生类自己定义的同名成员签名冲突，编译器优先使用派生类的版本（即「派生类版本覆盖引入版本」）。
- 同一个作用域内，using 声明引入的名字不能和已有名字冲突：

```cpp
int cout;              // 编译错误:和 using 声明冲突
using std::cout;
```

- 匿名命名空间配合 `using` 是控制符号可见性的惯用手法，但不要把 `static` 和匿名命名空间混用。

## 小结

| 写法 | 作用 |
|---|---|
| `using std::cout;` | 引入单个名字 |
| `using namespace std;` | 引入整个命名空间（慎用） |
| `using X = Y;` | 类型别名（替代 typedef，支持模板） |
| `using Base::Base;` | 继承构造函数 |
| `using Base::f;` | 引入基类重载集 / 调整访问权限 |
| `using std::swap; swap(a, b);` | 泛型惯用法：自定义版本优先，回退标准库 |
| `using enum E;` | C++20，枚举值进入作用域 |
| `using Mixins::f...;` | C++17，参数包展开引入多个基类成员 |
