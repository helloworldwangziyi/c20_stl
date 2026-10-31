# 第 7 章：运行时多态

运行时多态让同一份基类接口代码，根据对象的实际类型调用不同实现；它依靠虚函数以及
基类指针或引用，而不是对象切片后的基类值。

## 1. 静态类型与动态类型

```cpp
class shape {
public:
    virtual ~shape() = default;
    virtual double area() const = 0;
};

class rectangle final : public shape {
public:
    rectangle(double width, double height)
        : width_(width), height_(height)
    {
    }

    double area() const override { return width_ * height_; }

private:
    double width_;
    double height_;
};
```

```cpp
rectangle rect{2, 3};
const shape& ref = rect;
```

`ref` 的**静态类型**是 `const shape&`，编译器从这个声明看到的接口是 `shape`；它绑定对象
的**动态类型**是 `rectangle`。调用 `ref.area()` 时，虚函数机制选择 `rectangle` 的实现。

## 2. virtual 与动态绑定

没有 `virtual` 时，调用通常按静态类型决定；有 `virtual` 且通过指针或引用调用时，
才进行动态绑定：

```cpp
double twice_area(const shape& value)
{
    return value.area() * 2;
}
```

`twice_area` 不需要知道所有图形类型。新增 `circle`、`triangle` 后，只要它们正确实现
`shape` 接口，这个函数就能复用。

按值调用、直接操作基类值或发生对象切片时，不能得到同样的派生对象动态行为。

## 3. override：让编译器验证覆盖

```cpp
double area() const override;
```

`override` 表示“我要覆盖基类虚函数”。如果基类没有可覆盖的对应函数，编译器会报错。
它能抓住这类常见错误：

- 基类函数是 `area() const`，派生类却写成非 const。
- 参数类型写错，形成了一个新函数。
- 基类接口修改后，派生类函数不再匹配。

不要靠肉眼判断函数签名是否一致，派生类覆盖函数应尽量写 `override`。

## 4. final：停止继续覆盖或继承

`final` 可以放在虚函数上，禁止后续派生类继续覆盖：

```cpp
double area() const final;
```

也可以放在类上：

```cpp
class rectangle final : public shape {
};
```

此时不能再从 `rectangle` 派生。`final` 表达的是设计边界，不是每个虚函数或类都要加。

## 5. 纯虚函数与抽象类

```cpp
virtual double area() const = 0;
```

`= 0` 把函数声明为纯虚函数。含纯虚函数的类是抽象类，不能直接创建该类的对象：

```cpp
// shape s;   // 错误：shape 是抽象类
```

派生类必须实现所有仍需要的纯虚函数，否则它自己也仍是抽象类。抽象类常用来定义统一
接口，让调用者依赖能力而不是具体实现。

纯虚函数也可以有定义，但这不妨碍类保持抽象；初学阶段先掌握纯接口形式即可。

## 6. 为什么多态基类需要虚析构

如果通过基类指针删除派生对象，而基类析构函数不是虚函数，行为未定义：

```cpp
// base* p = new derived;
// delete p;              // 若 base 析构非 virtual，行为未定义
```

所以设计为“通过基类指针拥有并删除派生对象”的多态基类，通常应有公开虚析构：

```cpp
virtual ~shape() = default;
```

`unique_ptr<shape>` 使用默认删除器时同样依赖这一规则。若类不打算承担这种多态所有权
职责，就不应随意把它当成可删除的基类使用。

## 7. 虚函数表：作为实现模型理解

常见实现会在含虚函数的对象中保存指向虚函数表的指针，调用时根据动态类型找到对应
函数。这个模型有助于理解动态绑定，但 C++ 标准并不规定必须使用某种虚表布局。

不要手写代码依赖虚表地址、对象二进制布局或某个编译器的 ABI。语言保证的是虚调用
行为，不是具体实现细节。

虚调用可能增加间接调用成本并限制某些内联机会，但是否成为瓶颈仍要靠测量判断。

## 8. dynamic_cast、typeid 与 RTTI

RTTI 是运行时类型信息机制。对多态类型，`dynamic_cast` 可以检查实际对象是否可转换到
目标派生类型：

```cpp
void inspect(shape& value)
{
    if (auto* rect = dynamic_cast<rectangle*>(&value)) {
        // value 的确是 rectangle，可使用 rect
    }
}
```

指针形式失败返回空指针，引用形式失败会抛 `std::bad_cast`。`typeid` 可取得类型信息，
但依赖它大量分支往往说明接口没有表达真正需要的操作。

优先考虑把变化行为放进虚函数，而不是在每个调用点枚举所有派生类型。

## 9. 构造与析构期间的虚调用

对象构造和析构时，动态类型会随当前正在构造或销毁的层级变化。在基类构造函数中调用
虚函数，不会派发到尚未构造的派生类部分；在析构阶段也不会派发到已经销毁的派生部分。

因此不要用基类构造函数调用纯虚函数，或指望它完成派生类初始化。

## 10. 可编译示例

```cpp
#include <cassert>
#include <memory>

class shape {
public:
    virtual ~shape() = default;
    virtual double area() const = 0;
};

class rectangle final : public shape {
public:
    rectangle(double width, double height)
        : width_(width), height_(height)
    {
    }

    double area() const override { return width_ * height_; }

private:
    double width_;
    double height_;
};

double twice_area(const shape& value)
{
    return value.area() * 2;
}

int main()
{
    rectangle rect{2, 3};
    const shape& ref = rect;
    assert(ref.area() == 6);
    assert(twice_area(rect) == 12);

    std::unique_ptr<shape> owned = std::make_unique<rectangle>(4, 5);
    assert(owned->area() == 20);
}
```

同一个 `shape` 接口既处理栈上的 `rectangle`，也安全拥有并销毁动态分配的派生对象。

## 11. 本章要点

1. 静态类型来自声明，动态类型来自实际对象；虚调用通过指针或引用按动态类型分派。
2. `override` 检查覆盖，`final` 限制继续覆盖或继承。
3. 抽象类定义接口，多态基类需要正确的虚析构设计。
4. 优先使用虚函数表达变化行为，谨慎使用 RTTI 枚举具体类型。
