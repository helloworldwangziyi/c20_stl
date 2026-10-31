# 第 2 章：成员函数与类接口

成员函数决定外部如何与对象交互；`this`、const、静态成员和运算符重载，则决定这些
接口具体怎样表达。

## 1. this：当前调用对象的地址

非静态成员函数都有一个隐式参数 `this`，指向本次调用的对象：

```cpp
class counter {
public:
    void increment()
    {
        ++value_;              // 等价于 ++this->value_;
    }

private:
    int value_ = 0;
};
```

执行 `a.increment()` 时，`this` 指向 `a`；执行 `b.increment()` 时指向 `b`。
通常不需要显式写 `this->`，但在名字冲突或返回当前对象时很有用：

```cpp
class counter {
public:
    counter& set_value(int value)
    {
        this->value_ = value;  // 左侧是成员，右侧是参数
        return *this;          // 返回当前对象的左值引用
    }

private:
    int value_ = 0;
};
```

返回 `counter&` 可以支持 `c.set_value(1).increment()` 这样的连续调用。

## 2. const 成员函数：承诺不修改可观察状态

成员函数参数列表后的 `const`，表示该函数可以通过只读方式作用于当前对象：

```cpp
class point {
public:
    int x() const { return x_; }

private:
    int x_ = 0;
};
```

在 const 成员函数中，`this` 指向 const 对象，因此不能普通地修改非 `mutable` 成员。
const 对象可以调用 const 成员函数，不能调用非 const 成员函数：

```cpp
const point p;
p.x();          // 合法
// p.set_x(1);  // 若 set_x 不是 const，则非法
```

同一个函数可以提供 const 与非 const 两个重载，容器访问接口经常使用这种方式：

```cpp
int& at(std::size_t index);
const int& at(std::size_t index) const;
```

const 对象调用后者，避免得到可修改元素的引用。

## 3. 静态成员：属于类，而不是某个对象

静态数据成员由该类的所有对象共享，静态成员函数没有 `this`：

```cpp
class point {
public:
    static constexpr int dimension() { return 2; }

private:
    static inline int instance_count = 0;
};
```

调用静态成员函数不需要对象：

```cpp
static_assert(point::dimension() == 2);
```

`static inline` 数据成员从 C++17 起可以直接在类内定义。静态成员不适合保存每个对象
各自的状态，滥用也会把对象设计变成共享全局状态。

## 4. 运算符重载：让自定义类型使用自然语法

运算符重载本质上仍是有名字的函数。例如 C++20 可以默认生成相等比较：

```cpp
bool operator==(const point&) const = default;
```

于是可以写 `a == b`。常见形式包括：

- 成员形式：`a == b`、`a[index]`、`a += b`。
- 非成员形式：`os << value`，因为左操作数通常不是当前类。
- 函数调用形式：`function(arguments...)`，函数对象经常使用。

重载不能改变运算符的优先级和操作数个数，也不应赋予违反直觉的含义。例如不要拿
`+` 做删除元素，只因为它“能编译”。

## 5. friend：给特定外部函数或类开放访问权

友元函数不是成员函数，没有 `this`，但可以访问授予它权限的私有成员：

```cpp
class point {
    friend std::ostream& operator<<(std::ostream& os, const point& p);

private:
    int x_ = 0;
    int y_ = 0;
};
```

友元适合需要访问两个类内部细节的非成员接口，例如某些对称运算符。它不是随手绕过
封装的工具；如果公开只读接口已经足够，就不必增加友元。

## 6. 设计类接口的基本检查

一个清晰的接口通常满足：

- **职责明确**：函数名表达实际操作，不让一个函数偷偷承担无关任务。
- **const 正确**：只读操作标为 const，让 const 对象也能使用。
- **暴露必要能力**：不为了“以后可能有用”无限增加公有函数。
- **维护不变量**：任何公开操作结束后，对象仍处于有效状态。
- **返回类型表达所有权**：返回引用表示对象仍存在别处，返回值表示得到独立结果。

接口是调用者与实现者之间的契约，不只是把若干函数标成 `public`。

## 7. 可编译示例

```cpp
#include <cassert>
#include <sstream>
#include <string>

class point {
public:
    point() = default;
    point(int x, int y) : x_(x), y_(y) {}

    point& translate(int dx, int dy)
    {
        x_ += dx;
        y_ += dy;
        return *this;
    }

    int x() const { return x_; }
    int y() const { return y_; }

    static constexpr int dimension() { return 2; }
    bool operator==(const point&) const = default;

    friend std::ostream& operator<<(std::ostream& os, const point& p)
    {
        return os << '(' << p.x_ << ", " << p.y_ << ')';
    }

private:
    int x_ = 0;
    int y_ = 0;
};

int main()
{
    static_assert(point::dimension() == 2);

    point p{1, 2};
    p.translate(3, 4).translate(-1, 1);

    const point expected{3, 7};
    assert(p == expected);
    assert(expected.x() == 3);

    std::ostringstream out;
    out << p;
    assert(out.str() == "(3, 7)");
}
```

这里 `translate` 使用 `this` 返回当前对象，只读访问支持 const，`dimension` 属于类，
输出运算符则通过友元访问内部状态。

## 8. 本章要点

1. `this` 指向当前调用对象，静态成员函数没有 `this`。
2. const 成员函数是只读接口的重要组成部分，也可以参与重载。
3. 运算符重载要符合直觉，友元要有明确必要性的使用。
4. 好接口既方便调用者，也持续保护对象不变量。
