# 第 3 章：构造函数与对象初始化

构造函数的职责不是简单地“给成员赋值”，而是建立对象不变量，让对象一诞生就处于
可安全使用的状态。

## 1. 默认构造函数

不需要实参就能调用的构造函数是默认构造函数：

```cpp
class counter {
public:
    counter() = default;

private:
    int value_ = 0;
};
```

如果没有用户声明的构造函数，编译器通常会隐式提供默认构造函数；一旦声明了其他
构造函数，想保留无参构造就可以显式写 `= default`。

类内默认成员初始值也会参与默认初始化：

```cpp
class settings {
    int width_ = 80;
    int height_ = 24;
    bool visible_ = true;
};
```

`= default` 表示让编译器按语言的默认规则生成，不等于“什么都不做”：类类型成员仍会
被默认构造，已有类内初始值也会生效。

## 2. 带参数的构造函数

参数构造可以从调用者取得建立对象所需的信息：

```cpp
class rectangle {
public:
    rectangle(int width, int height)
        : width_(width), height_(height)
    {
    }

private:
    int width_;
    int height_;
};
```

调用 `rectangle r{2, 3};` 后，对象的两个维度在构造阶段就已经确定。如果尺寸必须为正，
应在构造过程中检查，而不是允许先造出无效对象再指望调用者补救。

## 3. 成员初始化列表：初始化发生在函数体之前

构造函数体执行前，基类和数据成员已经完成初始化。初始化列表表达的是“用什么构造
成员”，不是“进入函数体后再赋值”。

```cpp
class example {
public:
    example(int value) : value_(value) {}

private:
    const int value_;
};
```

以下成员尤其需要初始化列表：

- 没有默认构造函数的成员。
- 引用成员。
- 需要显式提供初值的 const 成员。
- 希望直接用参数构造、避免先默认构造再赋值的成员。

```cpp
class owner {
public:
    owner(int& value) : ref_(value), id_(42) {}

private:
    int& ref_;
    const int id_;
};
```

## 4. 初始化顺序由声明顺序决定

成员始终按照**类中声明的顺序**初始化，初始化列表里的书写顺序不会改变这一点：

```cpp
class order {
    int first_;
    int second_;

public:
    order() : second_(1), first_(2) {}
    // 实际仍先初始化 first_，再初始化 second_
};
```

如果一个成员的初始值依赖另一个成员，应先确认被依赖者已经初始化。最稳妥的做法是
让初始化列表顺序与声明顺序一致，并避免用尚未初始化的成员计算其他成员。

构造顺序可以概括为：基类先构造，成员按声明顺序构造，最后执行当前类的构造函数体。

## 5. 委托构造函数：复用另一份构造逻辑

委托构造函数把初始化工作交给同类的另一个构造函数：

```cpp
class rectangle {
public:
    explicit rectangle(int size) : rectangle(size, size) {}

    rectangle(int width, int height)
        : width_(width), height_(height)
    {
    }

private:
    int width_;
    int height_;
};
```

`rectangle{4}` 会委托给双参数构造函数。委托目标是初始化列表中唯一的初始化项，
目标构造完成后才执行委托构造函数的函数体。

## 6. explicit：阻止不需要的隐式转换

只有一个参数的构造函数很容易被当作转换构造函数：

```cpp
class distance {
public:
    explicit distance(int metres) : metres_(metres) {}

private:
    int metres_;
};
```

```cpp
distance a{10};      // 合法：直接构造
// distance b = 10;  // 错误：explicit 禁止复制初始化中的隐式转换
```

隐式转换可能让函数调用看起来方便，却也可能掩盖类型错误。单参数构造函数默认考虑
加 `explicit`，确实需要隐式转换时再移除。

## 7. = default 与 = delete

`= default` 请求编译器生成默认行为：

```cpp
struct point {
    int x = 0;
    int y = 0;

    point() = default;
};
```

`= delete` 明确禁止某个操作：

```cpp
class noncopyable {
public:
    noncopyable() = default;
    noncopyable(const noncopyable&) = delete;
    noncopyable& operator=(const noncopyable&) = delete;
};
```

删除函数会参与声明检查，但选择它会导致编译错误。它既能表达设计意图，也比把函数
藏到 private 更准确。

## 8. 可编译示例

```cpp
#include <cassert>
#include <stdexcept>

class rectangle {
    static int checked(int value)
    {
        if (value <= 0) {
            throw std::invalid_argument("dimensions must be positive");
        }
        return value;
    }

    int width_ = 1;
    int height_ = 1;

public:
    rectangle() = default;
    explicit rectangle(int size) : rectangle(size, size) {}

    rectangle(int width, int height)
        : width_(checked(width)), height_(checked(height))
    {
    }

    int area() const { return width_ * height_; }
};

int main()
{
    rectangle unit;
    rectangle square{4};
    rectangle rect{2, 3};

    assert(unit.area() == 1);
    assert(square.area() == 16);
    assert(rect.area() == 6);
}
```

所有公开构造路径都保证尺寸为正，调用者不可能通过这些接口造出零尺寸或负尺寸对象。

## 9. 本章要点

1. 构造函数要建立对象不变量，而不是只完成机械赋值。
2. 成员在构造函数体之前初始化，顺序取决于声明顺序。
3. 委托构造复用初始化逻辑，`explicit` 防止意外隐式转换。
4. `= default` 与 `= delete` 让编译器帮助表达接口意图。
