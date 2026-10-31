# 第 6 章：继承

继承让派生类包含基类子对象并复用基类接口；公有继承还表达“派生类可以当作基类使用”。
但继承不是普通代码复用按钮，组合往往更简单。

## 1. 公有继承与 is-a 关系

```cpp
class device {
public:
    explicit device(int id) : id_(id) {}
    int id() const { return id_; }

private:
    int id_;
};

class printer : public device {
public:
    explicit printer(int id) : device(id) {}
};
```

`printer` 是一种 `device`，因此公有继承合理。派生类对象中包含一个 `device` 基类
子对象，`device` 的公开接口也成为 `printer` 的公开接口：

```cpp
printer p{42};
int id = p.id();
```

派生类不能直接访问基类的 `private id_`，但仍可通过基类提供的公开或受保护接口使用它。

## 2. 派生类如何初始化基类

派生类构造函数必须先确保基类子对象被正确初始化：

```cpp
class printer : public device {
public:
    printer(int id, int pages)
        : device(id), pages_(pages)
    {
    }

private:
    int pages_;
};
```

如果基类没有默认构造函数，派生类通常必须在初始化列表中调用合适的基类构造函数。
不能像给普通成员赋值一样，在派生类构造函数体里“事后构造”基类。

## 3. 构造与析构顺序

完整对象的构造顺序是：

1. 基类子对象按继承规则构造。
2. 当前类的数据成员按声明顺序构造。
3. 执行当前类构造函数体。

析构严格反向进行：先执行派生类析构函数体，再销毁派生类成员，最后销毁基类子对象。
因此基类构造完成时派生部分还不存在，派生类析构开始时基类部分仍然存在。

## 4. public、protected、private 继承

继承说明符会调整基类成员在派生类中的可见性：

| 继承方式 | 基类 public 成员 | 基类 protected 成员 | 基类 private 成员 |
|---|---|---|---|
| `public` | 保持 public | 保持 protected | 派生类不可直接访问 |
| `protected` | 变为 protected | 保持 protected | 派生类不可直接访问 |
| `private` | 变为 private | 变为 private | 派生类不可直接访问 |

“不可直接访问”不等于基类私有成员不存在，它们仍是基类子对象的一部分，只是名字访问
受到限制。

`class derived : base` 默认是私有继承，`struct derived : base` 默认是公有继承。
实际代码应显式写出，避免让读者猜。

公有继承用于建立可替代关系；私有继承偶尔用于受控复用实现，但很多场景可以改成组合。
受保护继承更少用，初学阶段先能读懂即可。

## 5. protected 的作用与代价

`protected` 允许派生类访问，同时禁止普通外部代码通过对象访问：

```cpp
class base {
protected:
    void helper() {}
};

class derived : public base {
public:
    void run() { helper(); }
};
```

它扩大了类内部的暴露面：不仅当前类，未来所有派生类都可能依赖这些细节。设计不稳定
的基类时，protected 数据尤其容易形成耦合，优先考虑私有数据加受保护的函数接口。

## 6. 重载、隐藏、覆盖不是一回事

| 概念 | 发生位置 | 核心条件 |
|---|---|---|
| 重载 overload | 同一作用域 | 同名函数，参数列表不同 |
| 隐藏 hiding | 基类与派生类 | 派生类声明同名名字，遮蔽基类名字查找 |
| 覆盖 override | 基类与派生类 | 派生类重写基类虚函数，签名满足覆盖规则 |

名字隐藏经常造成意外：

```cpp
struct base {
    void f(int);
    void f(double);
};

struct derived : base {
    void f(int);
    // 不使用 using 时，base::f(double) 被名字隐藏
};
```

可以用 `using` 把基类重载集重新引入：

```cpp
struct derived : base {
    using base::f;
    void f(int);
};
```

虚函数覆盖需要 `virtual`、兼容的函数签名等条件，下一章用 `override` 让编译器检查。

## 7. 对象切片

把派生类对象按值复制到基类对象时，只保留基类子对象：

```cpp
struct base {
    int x = 0;
};

struct derived : base {
    int y = 0;
};

derived d;
d.x = 1;
d.y = 2;

base b = d;   // b 只有 base 部分，derived::y 被切掉
```

这叫对象切片。按值保存基类还会丢失动态类型信息，因此多态对象通常通过基类指针、
引用或合适的智能指针使用。

## 8. 组合与继承如何选择

**组合**表示“有一个”或“使用一个”：

```cpp
class engine {
};

class car {
    engine engine_;
};
```

汽车有发动机，但不是一种发动机，所以 `car` 不应继承 `engine`。判断时可以问：

- 派生对象是否真能在所有基类使用场景下替代基类？
- 我想复用的是接口和可替代性，还是只想少写几行代码？
- 把另一个对象作为成员，是否就能表达关系？

如果只是复用实现，优先考虑组合；如果需要统一接口和运行时替换，再考虑公有继承。

## 9. 可编译示例

```cpp
#include <cassert>
#include <string>
#include <utility>

class device {
public:
    explicit device(std::string id) : id_(std::move(id)) {}

    const std::string& id() const { return id_; }

private:
    std::string id_;
};

class printer : public device {
public:
    explicit printer(std::string id) : device(std::move(id)) {}

    void print() { ++printed_pages_; }
    int printed_pages() const { return printed_pages_; }

private:
    int printed_pages_ = 0;
};

int main()
{
    printer p{"printer-01"};
    p.print();
    p.print();

    assert(p.id() == "printer-01");
    assert(p.printed_pages() == 2);
}
```

这里复用了基类身份接口，同时把打印状态留在派生类中。示例没有通过基类指针删除对象，
也不涉及虚函数，运行时多态留到下一章。

## 10. 本章要点

1. 公有继承表达可替代关系，不只是复用代码。
2. 基类先构造、后析构，派生类必须在初始化列表中正确初始化基类。
3. 重载、隐藏和覆盖是三种不同机制，对象按值复制到基类会发生切片。
4. 组合通常比继承耦合更低，只在关系与接口都合适时使用公有继承。
