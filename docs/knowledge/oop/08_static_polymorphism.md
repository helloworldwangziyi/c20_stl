# 第 8 章：编译期多态与泛型

编译期多态在模板实例化和重载决议时确定调用，不需要共同的运行时基类；C++20 concepts
还能把原本隐含的模板要求写成明确约束。

## 1. 函数重载：编译期选择实现

```cpp
void print(int value);
void print(double value);

print(42);    // 选择 int 版本
print(3.14);  // 选择 double 版本
```

重载依据调用处的静态信息选择函数，不需要虚函数。它与覆盖不同：

- **重载**：同一作用域中的同名不同参数函数，编译期选择。
- **覆盖**：派生类实现基类虚函数，可在运行时按动态类型选择。
- **隐藏**：派生类同名声明影响基类名字查找，不是独立的多态机制。

## 2. 模板：按能力工作，而非按基类工作

```cpp
template <typename T>
double twice_area(const T& value)
{
    return value.area() * 2;
}
```

只要 `value.area()` 是合法表达式，并能参与后续计算，这个模板就可能工作。两个类型不必
继承同一个基类：

```cpp
struct square {
    double side;
    double area() const { return side * side; }
};

struct rectangle {
    double width;
    double height;
    double area() const { return width * height; }
};
```

这常被称为模板的“鸭式类型”：不关心类型名义上是什么，关心它是否提供所需表达式。
模板实例化后，具体调用通常在编译期确定。

## 3. 模板的接口原本是隐含的

没有约束时，`T` 的要求散落在模板实现里。错误可能在多层实例化后才暴露，调用者也难以
一眼看出需要哪些操作。

C++20 concepts 可以把要求写成接口契约：

```cpp
#include <concepts>

template <typename T>
concept area_type = requires(const T& value) {
    { value.area() } -> std::convertible_to<double>;
};
```

这里要求：

- 对 const 对象调用 `value.area()` 合法。
- 返回结果可以转换为 `double`。

使用概念约束模板：

```cpp
template <area_type T>
double twice_area(const T& value)
{
    return value.area() * 2;
}
```

不满足约束的类型会在重载选择中被排除，错误信息也通常更贴近真正的接口问题。

## 4. concepts 不是运行时接口

concept 不生成基类，不要求虚函数，也不会让不同类型的对象自动拥有共同运行时类型。
它只在编译期检查模板参数是否满足要求。

因此 `square` 和 `rectangle` 可以分别传给模板，但不能仅因为它们满足 `area_type`，就把
它们直接塞进同一个 `area_type*` 指针容器；`area_type` 根本不是一个运行时类型。

需要异构对象统一存储时，仍要考虑虚函数、类型擦除或 `std::variant` 等方案。

## 5. CRTP：编译期复用基类实现

CRTP（奇异递归模板模式）让派生类把自己作为模板参数交给基类：

```cpp
template <typename Derived>
class twice_runner {
public:
    void run_twice() const
    {
        const auto& self = static_cast<const Derived&>(*this);
        self.run();
        self.run();
    }
};

class job : public twice_runner<job> {
public:
    void run() const;
};
```

基类通过 `static_cast` 回到已知的派生类型，调用在编译期解析，不需要 `virtual`。
CRTP 适合静态复用接口和实现，但派生对象不能简单靠 `twice_runner<...>*` 组成统一的
运行时层级。

这种转换成立的前提是基类子对象确实属于对应的 `Derived`，错误使用 CRTP 类型会破坏
这一前提。

## 6. 编译期与运行时多态如何选择

| 需求 | 更自然的工具 |
|---|---|
| 不同类型在运行时统一替换 | 虚函数、抽象接口 |
| 同一指针容器保存不同派生对象 | 运行时多态或类型擦除 |
| 泛型算法处理无关类型 | 模板 + concepts |
| 编译期复用派生类能力 | CRTP 等静态技术 |
| 固定闭集类型，需要值存储 | 可考虑 `std::variant` |

模板提供更好的内联和静态优化机会，但也可能增加编译时间与生成代码体积；运行时多态
提供统一的二进制接口和动态替换能力。选择应由需求决定，而不是简单认为某一种永远更快。

## 7. 可编译示例

```cpp
#include <cassert>
#include <concepts>

template <typename T>
concept area_type = requires(const T& value) {
    { value.area() } -> std::convertible_to<double>;
};

template <area_type T>
double twice_area(const T& value)
{
    return value.area() * 2;
}

struct square {
    double side;
    double area() const { return side * side; }
};

template <typename Derived>
class twice_runner {
public:
    void run_twice() const
    {
        const auto& self = static_cast<const Derived&>(*this);
        self.run();
        self.run();
    }
};

class counter_job : public twice_runner<counter_job> {
public:
    explicit counter_job(int& count) : count_(count) {}
    void run() const { ++count_; }

private:
    int& count_;
};

int main()
{
    square value{3};
    assert(twice_area(value) == 18);

    int count = 0;
    counter_job job{count};
    job.run_twice();
    assert(count == 2);
}
```

`square` 不需要继承任何接口基类，`counter_job` 的两次调用也不需要虚分派；所有选择都在
编译期完成。

## 8. 本章要点

1. 重载和模板在编译期选择实现，虚函数覆盖支持运行时动态绑定。
2. 模板关注类型能力，concepts 把隐含要求变成明确约束。
3. CRTP 用派生类型作为模板参数复用实现，不等于运行时继承多态。
4. 根据异构存储、动态替换、泛型能力和工程成本选择方案。
